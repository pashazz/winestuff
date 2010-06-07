/*
WineStuff - library for manipulating WINE prefixes
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


#include "prefix.h"
#include <limits>
using namespace QtConcurrent;
Prefix::Prefix(QObject *parent, QString workdir, corelib *lib) :
		QObject(parent), _workdir (workdir),  s (new QSettings (_workdir + "/control", QSettings::IniFormat, this)), core (lib)
{
	db = QSqlDatabase::database();
_prefix = s->value("application/prefix").toString();
getPrefixPath();
//Настраиваем QProcessEnvironment
env = QProcessEnvironment::systemEnvironment();
env.insert("WINEDEBUG", "-all");
env.insert("WINEPREFIX", _path);
if (hasDBEntry())
	env.insert("WINE", wine());
}

void rp(QString path, QProcessEnvironment env)
{
	QProcess *p = new QProcess (0);
	QFileInfo file (path);
	if (file.exists())
		p->setWorkingDirectory(file.absolutePath());
	else
		p->setWorkingDirectory(env.value("WINEPREFIX"));
    p->setProcessEnvironment(env);
	p->start(path);
    p->waitForFinished(-1);
    delete p;
}

void Prefix::runProgram(QString exe)
{
QFuture <void> fProc = run(rp, QString("%1  \"%2\"").arg(wine()).arg(exe),  this->env);
//fProc.waitForFinished();
}

void Prefix::removePrefix()
{
	rp("rm -rf " + this->_path, QProcessEnvironment::systemEnvironment());
	/*
	  когда я пробовал использовать для этого removeDir из corelib
	  мой /home чуть не снесло
	  а все потому, что QFile/QDir плохо относится к симлинкам
	  так что только rm -rf. Я чуть не потерял свои данные
	  */
	if (isPreset())
		return;
	QSqlQuery q(db);
	q.prepare("DELETE FROM Apps WHERE prefix=:pr");
	q.bindValue(":pr", _prefix);
	if (!q.exec())
		qDebug() << "WARNING: Unable to execute query to delete Prefix";
}
QString Prefix::wine()
{
/// новый метод для получения wine из БД.
/// DO NOT USE IT BEFORE CALLING installFirstApplication (in first)
	QSqlQuery q (db);
	q.prepare("SELECT wine FROM Apps WHERE prefix=:prefix");
	q.bindValue(":prefix", _prefix);
	if (!q.exec())
	  {
		core->client()->error(tr("Database error"), tr("Failed to fetch wine"));
		return core->whichBin("wine");
			}
q.first();
QString wine  = q.value(0).toString();
if (wine.trimmed().isEmpty())
{
//	qDebug << "Can not get a wine, use system";
  wine = core->whichBin("wine");
}
env.insert("WINE", wine); //hack
return wine;
}

void Prefix::lauch_c() /// Use xdg-open instead of QDesktopServices.
 {
	rp ("xdg-open " + _path + "/dosdevices/c:", env);
}
void Prefix::lauchWinetricks(QStringList args)
{
    /// так как производится установка компонентов, чтобы юзер ничего не натворил, запускаем winetricks в том же потоке.
    QProcess *p = new QProcess (this);
    p->setProcessEnvironment(env);
    qDebug() << tr("engine: [prefix]: starting winetricks");
	foreach (QString arg, args)
	{
		p->start(core->whichBin("winetricks"), QStringList () << "-q" <<arg);
		p->waitForFinished();
	}
}


QString Prefix::standardExe()
{
    s->beginGroup("application");
    QString exe = s->value("exepath").toString();
    s->endGroup();
    return exe;
}

QString Prefix::name()
{
    //ищем файл .name в данном _workdir
    QFile file (_workdir + QDir::separator() + ".name." + QLocale::system().name() );
    if (file.exists())
    {
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString ret = file.readAll();
        file.close();
        return ret;
    }
    else
    {
 //cначала попробуем открыть просто .name
        file.setFileName(_workdir + QDir::separator() + ".name");
        if (file.exists())
        {
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QString ret = file.readAll();
            file.close();
            return ret;
        }
        else
        {
            QDir dir (_workdir);
            return dir.dirName();
        }
    }
}

QString Prefix::note()
{
    QString fileName;
    if (QFile::exists(_workdir + "/.note." + QLocale::system().name())) //читаем локализованное примечание
        fileName = _workdir + "/.note." + QLocale::system().name();
    else if (QFile::exists((_workdir + "/.note")))
        fileName = _workdir + "/.note";
    else
        return QString();
QFile file (fileName);
file.open(QIODevice::Text | QIODevice::ReadOnly);
QString note = file.readAll();
file.close();
return note;

}

bool Prefix::isPreset()
{
    return s->value("wine/preset").toBool();
}


bool Prefix::checkWineDistr()
{
	qDebug() << "checking wine... for " << name() ;
   /// проверяет дистрибутив Wine для префикса. Если проверка не удается, загружает дистрибутив заново.
	QFile file (_path + QDir::separator() + ".wine");
if (distr().isEmpty())
{
//TODO - удаление кастомного wine. если он более не нужен
	if (file.exists() && core->client()->questionDialog(tr("Wine outdated"), tr("Do you want to use system wine distribution for app %1?").arg(name())))
	{
		QString systemWine = core->whichBin("wine");
		//make "UPDATE" Sqlquery
		QSqlQuery q (db);
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
        QString wineUrl = downloadWine();
		if (wineUrl.isEmpty())
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

QString Prefix::downloadWine() {
    QString wineBinary;
	QString md5sum;
 if (!distr().isEmpty())
    {
	 QString wineDistr = distr();
	 //здесь запускаем процесс закачки и распаковки данного дистрибутива Wine
	 QString destination = core->wineDir()+ QString("/wines/") + prefixName();
	 QDir dir (core->wineDir()+ "/wines");
	 if (!dir.exists())
		 dir.mkdir(dir.path());
	 QString distrname =   core->downloadWine(wineDistr);
	 if (distrname.isEmpty())
		 return "";
	 if (!core->unpackWine(distrname, destination))
		 return "";

	 qDebug() << "wine distribution is" << wineDistr;
	 md5sum = getMD5();
	 if (!md5sum.isEmpty())
	 { //записываем сумму md5
		 qDebug() << "writing md5: " << md5sum;
		 writeMD5(md5sum);
	 }
	 return wineDistr;
 }
 else
 {
	 wineBinary = wine();
 }
    //если wineBinary все еще не установлен
return "";
}

bool Prefix::installFirstApplication()
{
	/// Проверяем Wine, загружаем его если надо, маркируем приложение как установленное и передаем управление движку'
	// Данная процедура не нужна, если мы используем презет; тут все дефолтно.
	if (s->value("application/prefix").isNull())
		return true; //skip this step.
QDir dir (_path);
if (!dir.exists())
	dir.mkpath(dir.path());

	if (!checkWineDistr())
		return false;
	//Db Working
	QSqlQuery q (db);
	q.prepare("INSERT INTO Apps (prefix, wineprefix, wine) VALUES (:prefix, :wineprefix, :wine)");
	q.bindValue(":prefix", _prefix );
	q.bindValue(":wineprefix", _path);
	QString wine;
	s->beginGroup("wine");
	QString distr = s->value("distr").toString();
	s->endGroup();
	if (distr.isEmpty())
	   {
		wine = corelib::whichBin("wine");
	}
	   else
	   {
		   wine = core->wineDir()+ "/wines/" + _prefix + "/usr/bin/wine";
	   }
	   q.bindValue(":wine", wine);
	   if (!q.exec())
	   {
		   core->client()->error(tr("Database error"), tr("Failed to execute query for application. See errors on console"));
		   qDebug() << "DB: Query error " << q.lastError().text();
		   return false;
	   }
	   return true;
}

bool Prefix::hasDBEntry()
{
	QSqlQuery q (db);

	q.prepare("SELECT * FROM Apps WHERE prefix=:pr");
	q.bindValue(":pr", _prefix);
	q.exec();
	return q.first();
}

void Prefix::setMemory()
{
	if (!s->value("wine/memory").toBool())
		return;
QTemporaryFile f (this);


	QTextStream stream (&f);
	f.open();
	stream << "\n";
	stream << "REGEDIT4\n";
	stream << "[HKEY_CURRENT_USER\\Software\\Wine\\Direct3D]";
	stream << "\n";
	stream << "\"VideoMemorySize\"=";
	stream << QString("\"%1\"").arg(core->videoMemory());
	stream << "\n";
	f.close();
QProcess p (this);
QStringList args;
args << "regedit";
args << f.fileName();
p.setProcessEnvironment(env);
p.start(wine(), args);
p.waitForFinished(-1);
f.remove();
}

void Prefix::getPrefixPath()
{
	if (isPreset() || _prefix.trimmed().isEmpty())
		return;

	if (hasDBEntry())
	{
		//пытаемся получить инфу из бд.
		QSqlQuery q(db);
		q.prepare("SELECT wineprefix FROM Apps WHERE prefix=:prefix");
		q.bindValue(":prefix", _prefix);
		if (q.exec())
		{
			q.first();
			_path=q.value(0).toString();
			if (!_path.isEmpty())
				return;
		}
		else
		{
			_path = core->wineDir() + QDir::separator() + _prefix;
			return;
		}
	}
		//ничего у нас не получилось, используем старый fallback-метод
		_path = core->wineDir() + QDir::separator() + _prefix;
	}

void Prefix::makeDesktopIcon(const QString &path, const QString &name)
{
	QFile file  (core->client()->desktopLocation() + QDir::separator() + name + ".desktop");
	QTextStream str (&file);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	str << "[Desktop Entry]\n";
	str << QString ("Name=%1\n").arg(name);
	str << "Type=Application\n";
	str << QString("Exec=env WINEPREFIX=\"%1\" winegame -r \"%2\"\n").arg(_path).arg(path);
	str << QString ("Icon=%1\n").arg(_workdir + QDir::separator() + "icon");
	str << QString ("Comment=%1\n").arg(note());
	str << QString("Categories=%1\n").arg("Game;"); //temporalily,
	file.close();
}



bool Prefix::runApplication(QString exe, QString diskroot, QString imageFile)
{
	if (hasDBEntry())
	{
		runProgram(exe);
		return true;
	}
	// ! Смотрим, определен ли префикс.
	if (s->value("application/prefix").isNull())
	{
		if (s->value("wine/preset").toBool())
		{
			//Таак, быстро отвязываем библиотечку от гуя
			QString prefixName;
			emit prefixNameNeed(prefixName); //если намереваемся ставить аппликуху, обязательно связываем этот сигнал со слотом, иначе краш и все такое.
			if (prefixName.isEmpty())
			{
				core->client()->error (tr("Installation error"),tr("Fatal error: Prefix name is empty."));
				return false;
			}
			else
			{
				//override constructor code
				_prefix = prefixName;
				_path = core->wineDir() + QDir::separator() + _prefix;
				env.insert("WINEPREFIX", _path);
			}	
		}
		else
		{
			core->client()->error (tr ("Packaging error"),tr("Fatal error: Package is broken."));
			return false;
		}
	}
	QString wineBin;
	if (!diskroot.isEmpty())
		env.insert("CDROOT", diskroot);

	if (QDir(_workdir + "/files").exists())
		env.insert("FILESDIR", _workdir + "/files");
	env.insert("WINEDEBUG", "-all");
	installFirstApplication();
	wineBin = wine(); //автоматически добавляет нужную запись в env.
	//запуск Winetricks
	lauchWinetricks(s->value("wine/components").toString().split(" ", QString::SkipEmptyParts));
	if (imageFile.isEmpty())
		makeWineCdrom(diskroot);
	else
		makeWineCdrom(diskroot, imageFile);

	/// TODO: работа по копированию файлов для последующей их установки с ЖД, если необходимо.
	/// мы будем поддерживать мультидисковые игры (если и будем) только с реальных дисков.
	//пока работаем так
	QProcess *proc = new QProcess (this);
	proc->setProcessEnvironment(env);
	proc->setWorkingDirectory(getExeWorkingDirectory(exe));
	if (QFile::exists(_workdir + "/preinst"))
	{
		proc->start ("\"" +_workdir + "/preinst\"");
		proc->waitForFinished(-1);
	}

	qDebug() << tr("engine: starting Windows program %1 with wine binary %2").arg(exe).arg(wineBin) << proc->workingDirectory();
	qDebug() << wineBin + " \"" + exe  +"\"" ;
	proc->start(wineBin + " \"" + exe  +"\"" );
	proc->waitForFinished(-1);

	//ну а теперь финальная часть, запуск postinst
	if (QFile::exists(_workdir + "/postinst"))
	{
	proc->start("\"" + _workdir + "/postinst\"");
	proc->waitForFinished(-1);
	makefix();
	}
	setMemory();

return true;
}

void Prefix::makeWineCdrom(const QString &path, const QString &device)
		//Path - путь к CDROM
		//Device - путь к реальному файлу устройства. /dev/cdrom по умолчанию, это может быть файл образа
{
	if (path.isEmpty())
		return;
	qDebug() << "engine: make DOS CD/DVD drive D" << path << "at" << device;
	QFile::link(path, _path + "/dosdevices/d:"); //Drive letter D: is hardcoded
	if (!device.isEmpty())
	QFile::link(device, _path + "/dosdevices/d::");
}

void Prefix::makefix()
{
	QFile file (_path + "/system.reg");
	QTextStream stream (&file);
   //сначала откроем реестр для чтения
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString registry = stream.readAll();
	file.close();
	registry.replace ("winebrowser.exe -nohome", "winebrowser.exe -nohome %1");
	file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	stream << registry;
	file.close();
}
QString Prefix::getExeWorkingDirectory(QString exe)
{
	QFileInfo info (exe);
	return info.absolutePath();
}

QString Prefix::setup()
{
	return s->value("application/setup").toString();
}

bool Prefix::isMulti()
{
	return s->value("disc/multicd").toBool();
}
 short int Prefix::discCount()
{
	 if (s->value("disc/count").toInt() > SHRT_MAX)
	 {
		 core->client()->error(tr("Package error"), "Olololo error");
		 return 0;
	 }
	return s->value("disc/count").toInt();
}

 void Prefix::writeMD5(QString md5sum)
 {
	 if (s->value("wine/nomd5", false).toBool())
		 return;

	 QFile file (_path +QDir::separator() + ".wine");
	 if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		 return;
	 qDebug() << "engine: writing md5 for " << name() << "sum " << md5sum;
	 QTextStream stream (&file);
	 stream << md5sum;
	 file.close();
 }
 QString Prefix::getMD5()
 {
	 if (s->value("wine/nomd5", false).toBool())
		 return "";
	//download MD5 file with QtNetwork
	 QUrl url = QUrl (distr()+ ".md5");
	 if (url.isEmpty())
		 return "";
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

 QString Prefix::distr()
 {
	 QUrl url (s->value("wine/distr").toString());
	 if (url.isEmpty())
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
