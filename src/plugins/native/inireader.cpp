/*
 libwst_native - native plugin for winestuff
Copyright (C) 2010 Pavel Zinin <pzinin@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "inireader.h"
#include "limits"

QString NativeReader::name()
{
	if (!_name.isEmpty())
		return _name;
	if (s->value("wine/preset").toBool())
	 core->client()->getText(tr("Template"), tr("Enter template name, for example 'CoolGame v3'"), _name);
	else
	_name = realName();

	return _name;
}

QString NativeReader::realName()
{
	QFile file (workdir() + QDir::separator() + ".name." + QLocale::system().name() );
	QString myName;
	if (file.exists())
	{
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		myName = file.readAll();
		file.close();
	}
	else
	{
 //cначала попробуем открыть просто .name
		file.setFileName(workdir() + QDir::separator() + ".name");
		if (file.exists())
		{
			file.open(QIODevice::ReadOnly | QIODevice::Text);
			myName = file.readAll();
			file.close();
		}
		else
		{
		  myName = QDir(workdir()).dirName();
		}
	}
	if (s->value("wine/preset").toBool())
		myName += tr(" [template]");
	return myName;
}

QString NativeReader::note()
{
	if (!_note.isEmpty())
		return _note;
	if (s->value ("wine/preset").toBool())
			core->client()->getText(tr("Template"), tr("Enter template`s note, for example 'It is a Cool Note'"), _note);
		else
			_note = realNote();

	return _note;
}
QString NativeReader::realNote ()
{
	QString fileName;
	QString myNote;
	if (QFile::exists(workdir() + "/.note." + QLocale::system().name())) //читаем локализованное примечание
		fileName = workdir() + "/.note." + QLocale::system().name();
	else if (QFile::exists((workdir() + "/.note")))
		fileName = workdir() + "/.note";
	else
		return QString();
	QFile file (fileName);
	file.open(QIODevice::Text | QIODevice::ReadOnly);
	myNote = file.readAll();
	file.close();
	return myNote;
}

bool NativeReader::checkWine()
{
	if (s->value("wine/preset").toBool())
		return true;

	qDebug() << "checking wine... for " << name() ;
   /// проверяет дистрибутив Wine для префикса. Если проверка не удается, загружает дистрибутив заново.
	QFile file (_prefix + QDir::separator() + ".wine");
	if (distr().isEmpty())
	{
		//TODO - удаление кастомного wine. если он более не нужен
		if (file.exists() && core->client()->questionDialog(tr("Wine outdated"), tr("Do you want to use system wine distribution for app %1?").arg(name())))
		{
			QString systemWine = core->whichBin("wine");
			//make "UPDATE" Sqlquery
			QSqlQuery q;
			q.prepare("UPDATE Apps SET wine=:wine WHERE prefix=:prefix");
			q.bindValue(":wine", systemWine);
			q.bindValue(":prefix", _prefix);
			if (!q.exec())
			{
				qDebug() << "engine: failed to execute query - " << q.lastError().text();
				return false;
			}
		}
		return true;
	}
	QTextStream stream (&file);

	if ((!file.exists()) || (!QFile::exists(wine())))
	{
		//Загружаем Wine
		if (!downloadWine())
		{
			qDebug() << "winechecker: empty url - guru meditation error";
			return false;
		}
	}
	else
	{
		//Открываем файл для чтения
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return false;

		QString installedWine = stream.readAll();
		file.close();
		QString md5sum =  getMD5();
		if (installedWine != md5sum)
		{
			//записываем MD5 в файл
			writeMD5(md5sum);
		}
	}
	qDebug() << "winechecker: done";
	return true;
}

QString NativeReader::wine()
{
if (s->value("wine/preset").toBool() || (distr().isEmpty()))
	return core->whichBin("wine");
else
	return core->wineDir() + "/wines/" + id  + "/usr/bin/wine";
}
/* todo: setup () */
QString NativeReader::prefixPath()
{
	if (!_prefix.isEmpty())
		return _prefix;
	if (s->value("wine/preset").toBool())
	{
		setPrefix:
		QString prsid;
		core->client()->getText(tr("Template"), tr("Enter template`s ID. F.e.  if you type 'mygame', then your app will be installed in %1.").arg(core->wineDir() + "/mygame/drive_c"), prsid);
		if (!prsid.isEmpty())
		{
			_prefix = core->wineDir() + QDir::separator() + prsid;
			if (QDir(_prefix).exists() || (!this->isIdValid(prsid)))
			{
				core->client()->error(tr("Enter an unique non-existent ID"), tr("This prefix is exists and/or invalid"));
				goto setPrefix;
			}
			id = prsid;
		}
		else
			_prefix = "";

	}
	else
	{
		_prefix = core->wineDir() + QDir::separator() + id;
	}
	return _prefix;
}
QStringList NativeReader::components()
{
	return s->value ("wine/components").toString().split(" ", QString::SkipEmptyParts);
}


QString NativeReader::icon()
{
	QDir dir ("packages:" + id);
	if (dir.exists("icon"))
		return dir.absoluteFilePath("icon");
	else
		return "";
}


bool NativeReader::downloadWine() {
	QString md5sum;
 if (!distr().isEmpty())
	{
	 QString wineDistr = distr();
	 qDebug() << "wine distrib" << wineDistr;
	 //здесь запускаем процесс закачки и распаковки данного дистрибутива Wine
	 QString destination = core->wineDir()+"/wines/" + id;
	 QDir dir (core->wineDir()+ "/wines");
	 if (!dir.exists())
		 dir.mkdir(dir.path());
	 QString distrname =   core->downloadWine(wineDistr);
	 if (distrname.isEmpty())
	 {
		 core->client()->error(tr("Unable to download Wine"), tr("Error info: Failed to download Wine for %1" ).arg(name()));
		 return false;
	 }
	 else if (distrname == "CANCEL")
	 {
		 return false;
	 }
	 if (!core->unpackWine(distrname, destination))
	 {
		 core->client()->error(tr("Unable to unpack Wine"), tr("Error info: Failed to unpack %1 into %2").arg(distrname).arg(destination));
		 return false;
	 }
	 md5sum = getMD5();
	 if (!md5sum.isEmpty())
	 { //записываем сумму md5
		 qDebug() << "writing md5: " << md5sum;
		 writeMD5(md5sum);
	 }
	 QFile::remove(distrname);
 }
 return true;
}

QString NativeReader::distr()
{
	QUrl url (s->value("wine/distr").toString());
	if (url.isEmpty() || (!url.isValid()))
		return "";
	if (url.scheme() == "wg")
	{
		//Get file from winegame-project.ru
		const QString site = "http://winegame-project.ru/wine"; //yet hardcoded
		QString realUrl = site + "/" + core->unixSystem() + "/" + url.path();
		return realUrl;
	}
	else
		return url.toString();
}

void NativeReader::writeMD5(const QString &md5sum)
{
	if (s->value("wine/nomd5", false).toBool())
		return;

	QFile file (prefixPath() +QDir::separator() + ".wine");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		return;
	qDebug() << "engine: writing md5 for " << name() << "sum " << md5sum;
	QTextStream stream (&file);
	stream << md5sum;
	file.close();
}
QString NativeReader::getMD5()
{
	if (s->value("wine/nomd5", false).toBool())
		return "";
   //download MD5 file with QtNetwork
	if (distr().isEmpty())
		return "";
	QUrl url = QUrl (distr()+ ".md5");

	QEventLoop loop;
	QNetworkAccessManager *manager = new QNetworkAccessManager (this);
	QNetworkRequest req; //request для Url
	req.setUrl(QUrl(url));
	req.setRawHeader("User-Agent", "Winegame-Browser 0.1");
	QNetworkReply *reply = manager->get(req);
	connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
	QTextStream stream (reply);
	QString md5 = stream.readAll();
	qDebug() << "md5sum:" << md5 << "for app" << name();
	return md5;
}

bool NativeReader::isMulticd()
{
	return s->value("disc/multicd").toBool();
}
 short int NativeReader::discCount()
{
	 if (s->value("disc/count").toInt() > SHRT_MAX)
	 {
		 core->client()->error(tr("Package error"), "Olololo error");
		 return 0;
	 }
	return s->value("disc/count").toInt();
}

 bool NativeReader::needToSetMemory()
 {
	 return s->value("wine/memory").toBool();
 }

 QStringList NativeReader::locales()
 {
	 if (s->value("wine/preset").toBool())
		 return QStringList ("C");
	 QDir dir ("packages:" + id);
	 QStringList loc;
	 QStringList  filters = QStringList () << ".name*" << ".note*";
	 foreach (QFileInfo file, dir.entryInfoList(filters, QDir::Files | QDir::Readable))
	 {
		loc << file.suffix();
	 }
	 loc.removeDuplicates();
	 loc.prepend("C");
	 return loc;
 }

/*!
  *  Get name and note for given locale.
  * List of locales can be got with locales()
  */

 Name NativeReader::nameForLang(QString locale)
 {
	 Name names;
	 if (!s->value("wine/preset").toBool())
	 {
		 QString nameFile, noteFile;
		 if (locale == "C")
			 nameFile = QString ("packages:%1/.name").arg(id);
		 else
		 {
			 nameFile = QString ("packages:%1/.name.%2").arg(id).arg(locale);
	 if (!QFile::exists(nameFile))
		 nameFile = QString ("packages:%1/.name").arg(id);
 }
	 if (QFile::exists(nameFile))
		 {
		 QFile file (nameFile);
		 if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		 {
			 names.first = file.readAll();
			 file.close();
		 }
		 else
			 names.first = id;
	 }
	 else
		 names.first = id;
// The same for notefile
	 if (locale == "C")
		 noteFile = 	 nameFile = QString ("packages:%1/.note").arg(id);
	 else
	 {
		 noteFile = QString("packages:%1/.note.%2").arg(id).arg(locale);
		 if (!QFile::exists(noteFile))
			 nameFile = QString ("packages:%1/.note").arg(id);
	 }
	 if (QFile::exists(noteFile))
	 {
		 QFile file (noteFile);
		 if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		 {
			 names.second = file.readAll();
			 file.close();
		 }
 }
 }
	 else
	 {
		names.first = name();
		names.second = note();
	 }
	 return names;
 }

 QString NativeReader::workdir()
 {
	 return QDir ("packages:" + id).absolutePath();
 }

 QStringList NativeReader::discFileList (const QString &disc)
 {
     QFile file (QString ("%1/cdrom.d/%2").arg (workdir ()).arg (disc));
     QTextStream stream (&file);
     if (!file.open (QIODevice::ReadOnly | QIODevice::Text))
	 return QStringList();

     QStringList list;
     while (!stream.atEnd ())
	 list << stream.readLine ();
     return list;
 }

 QStringList NativeReader::availableDiscs ()
 {
     QDir dir (workdir () + "/cdrom.d");
     return dir.entryList (QDir::Files | QDir::Readable);
 }

 bool NativeReader::detectApp(QString path)
 {
	 //Get workdir (dir of winegame package) (detecting code here)
	 //If not detected, return empty string
	 if (path.isEmpty())
		 return false;
	 if (!QFileInfo(path).exists())
		 return false;
	 QString diskPath;
	 if (QFileInfo(path).isFile())
		 diskPath = core->mountDir();
	 else
		 diskPath = path;
	 QDir disc (diskPath);
	 QStringList disclist = disc.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
	 foreach (QString disc, availableDiscs())
	 {
		 QStringList list (discFileList(disc));
		 //посчитаем кол-во эквивалентов
		 int i = 0;
		 foreach (QString str, list)
		 {
			 if (disclist.contains(str, Qt::CaseInsensitive))
				 i++;
		 }
			 if (i == disclist.count())
		 {
				 return true;
		 }
		  return false;
	  }
 }

 Prefix::ApplicationType NativeReader::type()
 {
	 Prefix::ApplicationType type;
	 const QString key = "application/category";
	 if (s->value(key).toString() == "application")
		 type = Prefix::Application;
	 else if (s->value(key).toString() == "arcade")
		 type = Prefix::Arcade;
	 else if (s->value(key).toString() == "sports")
		 type = Prefix::Sports;
	 else if (s->value(key).toString() == "fps")
		 type = Prefix::FistPersonShooter;
	 else if (s->value(key).toString() == "strategy")
		 type = Prefix::Strategy;
	 else if (s->value(key).toString() == "rpg")
		 type = Prefix::Roleplaying;
	 else if (s->value(key).toString() == "action")
		 type = Prefix::Action;
	 else
		 type = Prefix::Other;
	 return type;
 }

 bool NativeReader::setup(const QString &file)
 {
	 QProcess *p = new QProcess (this);
	 Prefix *pref = prefix();
	 p->setProcessEnvironment(pref->environment());
	  if (file.isEmpty())
		return false;
 QString exe = executable(file);
 QDir dir ("packages:" + id);
 QString preinst = dir.absoluteFilePath("preinst");
 if (QFile(preinst).exists())
	 core->runGenericProcess(p, preinst, tr("Running pre-installation trigger"));
 //собсно наш exe
 pref->runApplication(exe, "", true); //выводим мод. диалог
 //теперь postinst
 QString postinst = dir.absoluteFilePath("posinst");
if (QFile(postinst).exists())
	core->runGenericProcess(p, postinst, tr("Running post-installation trigger"));
 }

 Prefix * NativeReader::prefix()
 {
	 Prefix *prefix = new Prefix (this, core);
	 prefix->setName(name());
	 prefix->setNote(note());
	 prefix->setPath(prefixPath());
	 prefix->setWine(wine());
	 if ((!!_cdroot.isEmpty()) && (!_device.isEmpty()))
	 {
		 prefix->setDiscAttributes(_cdroot, _device);
	 }
	 prefix->setType(type());
	 return prefix;
 }

 void NativeReader::setDvd(const QString &device, const QString &path)
 {
	 if (device.isEmpty())
		 _device = "/dev/cdrom";
	 else
		 _device = device;
	 _cdroot = path;
 }

 QString NativeReader::defaultWine()
 {
	 if (s->value("wine/distr").isNull())
		 return core->whichBin("wine");
	 else
		 return QString ("winedir:wines/%1/usr/bin/wine").arg(id);
 }