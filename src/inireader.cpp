/*
    Winegame - small utility to prepare Wine and install win32 apps in it.
    Copyright (C) 2010 Pavel Zinin <pzinin@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/


#include "inireader.h"
#include "limits"

QString SourceReader::name()
{
	if (!_name.isEmpty())
		return _name;
	if (s->value("wine/preset").toBool())
	{
		emit presetNameNeed(_name);
	}
	else
	{
	//ищем файл .name в данном workdir()
	QFile file (workdir() + QDir::separator() + ".name." + QLocale::system().name() );
	if (file.exists())
	{
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		_name = file.readAll();
		file.close();
	}
	else
	{
		file.setFileName(workdir() + QDir::separator() + ".name");
		if (file.exists())
		{
			file.open(QIODevice::ReadOnly | QIODevice::Text);
			_name = file.readAll();
			file.close();
		}
		else
		{
		  _name = QDir(workdir()).dirName();
		}
	}
}
	return _name;
}

QString SourceReader::realName()
{
	QFile file (workdir() + QDir::separator() + ".name." + QLocale::system().name() );
	if (file.exists())
	{
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		_name = file.readAll();
		file.close();
	}
	else
	{
 //cначала попробуем открыть просто .name
		file.setFileName(workdir() + QDir::separator() + ".name");
		if (file.exists())
		{
			file.open(QIODevice::ReadOnly | QIODevice::Text);
			_name = file.readAll();
			file.close();
		}
		else
		{
		  _name = QDir(workdir()).dirName();
		}
	}
	return _name;
}


QString SourceReader::note()
{
	if (!_note.isEmpty())
		return _note;
	if (s->value ("wine/preset").toBool())
	{
		emit presetNoteNeed(_note);
	}
	else
	{
		QString fileName;
		if (QFile::exists(workdir() + "/.note." + QLocale::system().name())) //читаем локализованное примечание
			fileName = workdir() + "/.note." + QLocale::system().name();
		else if (QFile::exists((workdir() + "/.note")))
			fileName = workdir() + "/.note";
		else
			return QString();
		QFile file (fileName);
		file.open(QIODevice::Text | QIODevice::ReadOnly);
		_note = file.readAll();
		file.close();
	}
	return _note;
}
QString SourceReader::realNote ()
{
	QString fileName;
	if (QFile::exists(workdir() + "/.note." + QLocale::system().name())) //читаем локализованное примечание
		fileName = workdir() + "/.note." + QLocale::system().name();
	else if (QFile::exists((workdir() + "/.note")))
		fileName = workdir() + "/.note";
	else
		return QString();
	QFile file (fileName);
	file.open(QIODevice::Text | QIODevice::ReadOnly);
	_note = file.readAll();
	file.close();
return _note;
}


 QStringList SourceReader::configurations (const QStringList &directories)
 {
	 QStringList list;
	 foreach (QString directory, directories)
	 {
	 foreach (QString dir, QDir(directory).entryList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot))
	 {
			 list << dir;
	 }
 }
	 return list;
 }
bool SourceReader::checkWine()
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

QString SourceReader::wine()
{
if (s->value("wine/preset").toBool() || (distr().isEmpty()))
	return core->whichBin("wine");
else
	return core->wineDir() + "/wines/" + id  + "/usr/bin/wine";
}
QString SourceReader::setup()
{
	return s->value("application/setup").toString();
}
QString SourceReader::prefixPath()
{
	if (!_prefix.isEmpty())
		return _prefix;
	if (s->value("wine/preset").toBool())
	{
		QString prsid;
		emit presetPrefixNeed(prsid);
		if (!prsid.isEmpty())
		_prefix = core->wineDir() + QDir::separator() + prsid;
		else
			_prefix = "";
	}
	else
	{
		_prefix = core->wineDir() + QDir::separator() + id;
	}
	return _prefix;
}
QStringList SourceReader::components()
{
	return s->value ("wine/components").toString().split(" ", QString::SkipEmptyParts);
}
QString SourceReader::filesDirectory()
{
	QDir dir ("packages:" + id);
	if (dir.exists("files"))
		return dir.absoluteFilePath("files");
	else
		return "";
}

QString SourceReader::icon()
{
	QDir dir ("packages:" + id);
	if (dir.exists("icon"))
		return dir.absoluteFilePath("icon");
	else
		return "";
}
QString SourceReader::preinstCommand()
{
	QDir dir ("packages:" + id);
	if (dir.exists("preinst"))
	{
		QFileInfo info (dir.absoluteFilePath("preinst"));
		if (info.isExecutable())
			return info.absoluteFilePath();
	}
	return "";
}

QString SourceReader::postinstCommand()
{
	QDir dir ("packages:" + id);
	if (dir.exists("postinst"))
	{
		QFileInfo info (dir.absoluteFilePath("postinst"));
		if (info.isExecutable())
			return info.absoluteFilePath();
	}
	return "";
}

bool SourceReader::downloadWine() {
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

QString SourceReader::distr()
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

void SourceReader::writeMD5(const QString &md5sum)
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
QString SourceReader::getMD5()
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

bool SourceReader::isMulticd()
{
	return s->value("disc/multicd").toBool();
}
 short int SourceReader::discCount()
{
	 if (s->value("disc/count").toInt() > SHRT_MAX)
	 {
		 core->client()->error(tr("Package error"), "Olololo error");
		 return 0;
	 }
	return s->value("disc/count").toInt();
}

 bool SourceReader::needToSetMemory()
 {
	 return s->value("wine/memory").toBool();
 }

 QStringList SourceReader::locales()
 {
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

 Name SourceReader::nameForLang(QString locale)
 {
	 Name names;
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
	 return names;
 }

 QString SourceReader::workdir()
 {
	 return QDir ("packages:" + id).absolutePath();
 }

 bool SourceReader::updateWines (const QStringList &prefixes, corelib *core)
 {
	 QStringList confs = SourceReader::configurations(core->packageDirs());
	 foreach (QString str, prefixes)
	 {
		 if (!confs.contains(str))
			 continue;
		 SourceReader reader (str, core, 0);
		bool res = reader.checkWine();
		if (!res)
			return false;
	 }
	 return true;
 }

 QString SourceReader::defaultWine(const QString &id)
 {
	 return QString ("winedir:wines/%1/usr/bin/wine").arg(id);
 }

 QStringList SourceReader::discFileList (const QString &disc)
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

 QStringList SourceReader::availableDiscs ()
 {
     QDir dir (workdir () + "/cdrom.d");
     return dir.entryList (QDir::Files | QDir::Readable);
 }

 bool SourceReader::preset()
 {
	 return s->value("wine/preset", false).toBool();
 }


 bool SourceReader::detectApp(QString path)
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

 Prefix::ApplicationType SourceReader::type()
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
