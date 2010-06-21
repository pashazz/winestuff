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


#include "corelib.h"
//The core of WineGame. Commonly used func.
corelib::corelib(QObject *parent, UiClient *client)
	:QObject(parent), ui (client)
{
	//Init Settings object
	settings = new QSettings (config(), QSettings::IniFormat, this);
	QDir::setSearchPaths("packages", packageDirs());
	QDir::setSearchPaths("winedir", QStringList(wineDir()));
	//Init translations object
#ifdef Q_WS_X11
QProcess p (this);
p.start("uname");
p.waitForFinished();
system = p.readAllStandardOutput().toLower().trimmed().replace('\n', "");
#endif
}

QString corelib::whichBin(QString bin) {
    QProcess p (0);
    p.start("which", QStringList (bin));
    p.waitForFinished(-1);
    QString path = QString(p.readAll()).trimmed();
    path.remove('\n');
	if (QFile::exists(path))
		return path;
	else
		return "";
}
void corelib::init(const QString &configPath, const QString &dbConnectionName)
{
	if (!QFile::exists(whichBin("wine")))
	{
		qApp->exit(-4);
	}
	initconf(configPath);
//скачаем пакеты
	if (!syncPackages())
	{
		ui->error(tr("Initialzation error"), tr("Unable to download packages"));
		qApp->exit(-7);
	}

	bool isMakeDb;
	isMakeDb = (!QFile::exists(wineDir() + "/installed.db"));
	if (dbConnectionName.isEmpty())
		db = QSqlDatabase::addDatabase("QSQLITE");
	else
		db = QSqlDatabase::addDatabase("QSQLITE", dbConnectionName);
	db.setDatabaseName(wineDir() + "/installed.db");
	if (!db.open()){
		ui->error(tr("Database error"), tr("Failed to open database for storing installed applications. See errors on console"));
		qDebug() << "DB: error: " << db.lastError().text();
		qApp->exit(-24);
	}
	if (isMakeDb)
		initDb();
}


bool corelib::unpackWine (QString distr, QString destination)
{
     QDir dir (destination);
     if (!dir.exists())
         dir.mkdir(dir.path());
 QProcess *proc = new QProcess (0); //не забываем удалять
 proc->setWorkingDirectory(destination);
 QString unpackLine =  QString ("tar xvpf %1 -C %2").arg(distr).arg(destination);
runGenericProcess(proc, unpackLine, tr("Unpacking wine...."));
 qDebug() <<QString("engine: Wine distribution %1 unpacked to %2").arg(distr).arg(destination);
 return proc->exitCode() == 0 ? true : false;
	 }

QString corelib::downloadWine(QString url) //TODO: проверка на ошибки.
{
	downloadExitCode = true;
    QUrl myurl = QUrl(url);
    QFileInfo inf (myurl.path());
	QString wineFileName = QDir::tempPath() + QDir::separator() + inf.fileName();
	if (QFileInfo(wineFileName).exists())
		return wineFileName;
	ui->showNotify(tr("Don`t worry!"), tr("Now WineGame will download some files, that will need for get your applicaton running"));
     QEventLoop loop;
QNetworkAccessManager *manager = new QNetworkAccessManager (this);
QNetworkRequest req; //request для Url
req.setUrl(QUrl(url));
req.setRawHeader("User-Agent", "Winegame-Browser 0.1");
QNetworkReply *reply = manager->get(req);
currentReply = reply;
connect (reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(setRange(qint64,qint64)));
connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
connect (reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT (error(QNetworkReply::NetworkError)));
ui->showProgressBar(tr("Downloading Wine..."), SLOT(cancelCurrentOperation()), this);
ui->progressText(tr("Downloading wine... %1").arg(url));
loop.exec();
ui->endProgress();
if (reply->error() == QNetworkReply::OperationCanceledError)
	return "CANCEL";
QByteArray buffer = reply->readAll();
//Get MD5 sum info...
//do not provide error info..
qDebug() << "Download done...";
QFile file (wineFileName);
qDebug() << "Writing into file: " << file.fileName();
if (file.open(QIODevice::WriteOnly))
{
		file.write(buffer);
		file.close();
	}
else
{
	qDebug() << "engine: error open file (WINEDISTR):" << file.errorString();
        return "";
}


return downloadExitCode ? file.fileName() : "";
}


void corelib::error(QNetworkReply::NetworkError error)
{
	if  (error == QNetworkReply::NoError || error == QNetworkReply::OperationCanceledError)
    {
       return;
   }
    else
    {
		QString errstr;
		switch (error)
		{
		case QNetworkReply::ConnectionRefusedError:
			errstr = tr("Connection refused");
			break;
		case QNetworkReply::RemoteHostClosedError:
			errstr = tr("Remote host closed connection");
			break;
		case QNetworkReply::HostNotFoundError:
			errstr = tr("Host not found");
			break;
		case QNetworkReply::TimeoutError:
			errstr = tr("Connection timeout");
			break;
		case QNetworkReply::OperationCanceledError:
			errstr = tr("Operation Canceled");
			break;
		case QNetworkReply::SslHandshakeFailedError:
			errstr = tr("SSL error");
			break;
		case QNetworkReply::ProxyConnectionRefusedError:
		case QNetworkReply::ProxyAuthenticationRequiredError:
		case QNetworkReply::ProxyTimeoutError:
		case QNetworkReply::ProxyNotFoundError:
		case QNetworkReply::ProxyConnectionClosedError:
		case QNetworkReply::UnknownProxyError:
			errstr = tr("Proxy server error");
			break;
		case QNetworkReply::ContentAccessDenied:
			errstr = tr("Acess denied");
			break;
		case QNetworkReply::ContentNotFoundError:
		case QNetworkReply::ContentOperationNotPermittedError:
			errstr = tr("File not found/Acess not permitted");
			break;
		case QNetworkReply::AuthenticationRequiredError:
			errstr = tr ("Authentication Required");
			break;
		case QNetworkReply::ContentReSendError:
			errstr = tr("Content resend error");
			break;
		default:
			errstr = tr("Unknown error");
			break;
		}
		ui->error(tr("Network error"), tr("Something went wrong! %1.").arg(errstr));
	  downloadExitCode = false;
    }
}

void corelib::setRange(qint64 aval, qint64 total)
{
    int kbAval = aval;
    int kbTotal = total;
ui->progressRange(kbAval, kbTotal);
}

void corelib::exitApp()
{
	ui->error(tr("Critical error"), tr("Wine distribution not downloaded, so exit installation."));
	downloadExitCode = false;
}

bool corelib::checkPrefixName(QString prefix)
{
    //пока что тут у нас проверяется на пробелы.
    if (prefix.contains(' '))
        return false;
	if (prefix == "wines") //зарезервировано для вайнов
		return false;
	if (prefix == "wineversion") //также
		return false;
 return true;
}

void corelib::runSingleExe(QStringList exe)
{
	QString wineprefix = QProcessEnvironment::systemEnvironment().value("WINEPREFIX");
	if (wineprefix.isEmpty())
	{
		qDebug() << "winegame: Wineprefix not set, exiting.";
		return;
	}
	// Выбираем бинарник Wine по данному WINEPREFIX
	QSqlQuery q(db);
	q.prepare("SELECT wine FROM Apps WHERE wineprefix=:pr");
	q.bindValue(":pr", wineprefix);
	q.exec();
	q.first();
	QString wine =q.value(0).toString();
	if (wine.isEmpty())
	{
	  qDebug() << "Wine from WineGame not found, use default";
		wine = whichBin("wine");
	}
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("WINEDEBUG", "-all");
 QProcess proc (this);
 proc.start(wine, exe);
 proc.waitForFinished(-1);
}

bool corelib::initconf(const QString &configPath)
{
	//Init our configuration.
	if (settings->value("VideoMemory").isNull())
	{
	int mem = ui->getVideoMemory();
	setVideoMemory(mem, true);
}
	QDir dir (configPath);
	setWineDir(dir.absoluteFilePath("windows"), true);
	setMountDir(dir.absoluteFilePath("mounts"), true);
	setDiscDir(dir.absoluteFilePath("disc"), true);
	setPackageDirs(QStringList(dir.absoluteFilePath("packages")), true);
	setSyncMirrors(QStringList("http://winegame-project.ru/winepkg"), true);
	if (QDir(discDir()).exists())
	{
	//подчищаем discDir
	ui->showProgressBar(tr("Cleaning up"));
	removeDir(discDir());
	ui->endProgress();
}
	//check if dirs exists
QStringList paths = QStringList () << wineDir() << mountDir() /*<< discDir()*/ ;
foreach (QString path, paths)
{
	QDir dir (path);
	if (!dir.exists())
		dir.mkpath(dir.path());
}
return true;
}

QString corelib::wineDir() {
	return settings->value("WineDir").toString();
}
QStringList corelib::packageDirs() {
	return settings->value("PackageDir").toString().split(';', QString::SkipEmptyParts);
}
QString corelib::mountDir() {
	return settings->value("MountDir").toString();
}
void corelib::setWineDir(QString dir, bool isempty)
{
	if (isempty && (!wineDir().isEmpty()))
	{
		//выставляем isempty в false, чтобы форсировать установку настройки....
		QFileInfo myDir (wineDir());
		if ((!myDir.exists()) || (!myDir.isWritable()))
			isempty = false;
	}
	setConfigValue("WineDir", dir, isempty);
}
void corelib::setPackageDirs(const QStringList &dirs, bool isempty)
{
	if (isempty && (!packageDirs().isEmpty()))
	{
		foreach (QString dir, packageDirs()){
		QFileInfo mdir (dir);
		if ((!mdir.exists()) || (!mdir.isWritable()))
			isempty = false;
	}
	}
	setConfigValue("PackageDir", dirs.join(";"), isempty);
}
void corelib::setMountDir(QString dir, bool isempty)
{
	if (isempty && (!mountDir().isEmpty()))
	{
		QFileInfo myDir (mountDir());
		if (forceFuseiso())
		{
		if ((!myDir.exists()) || (!myDir.isWritable()))
			isempty = false;
	}
		else
		{
			if ((!myDir.exists()) || (!myDir.isReadable()))
				isempty = false;
		}
	}
	setConfigValue("MountDir", dir, isempty);
}
void corelib::setVideoMemory(int memory, bool isempty)
{
	int oldMemory = settings->value("VideoMemory").toInt();
	if (oldMemory == memory)
		return;
	setConfigValue("VideoMemory", memory, isempty);
	settings->sync(); //we need to force sync
	//Sync all videomemory entries
/*	foreach (QString d, packageDirs())
	{
		QDir dir(d);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Readable))
	{
		//construct prefix obj
		Prefix *prefix = new Prefix (this->parent(), info.absoluteFilePath(), this);
		if (prefix->hasDBEntry())
		{
			prefix->checkWineDistr();
			prefix->setMemory();
		}
	}
}
*/ //todo: port it to new architecture
}
QString corelib::videoMemory()
{
	return settings->value("VideoMemory").toString();
}

QString corelib::autorun(QString diskRoot)
{
	QStringList autorunNames;
	autorunNames.append("autorun.inf");
	autorunNames.append("Autorun.inf");
	autorunNames.append("AUTORUN.INF");
	autorunNames.append("AutoRun.inf");
 QDir dir (diskRoot);
 qDebug() << "autorun: diskroot" << diskRoot;
 foreach (QString fileName,  dir.entryList(QDir::Files | QDir::Readable))
 {
	 if (autorunNames.contains(fileName, Qt::CaseSensitive))
	 {
		 qDebug()  << diskRoot + QDir::separator() + fileName;
		 return diskRoot + QDir::separator() + fileName;
	 }
 }
 return "";
}


corelib::~corelib()
{
	QSqlDatabase db = QSqlDatabase::database();
	db.close();

}

QString corelib::getSudoProg()
{
	QStringList programs = QStringList () << "kdesu" << "gksu" << "xdg-su";
	foreach (QString str, programs)
	{
		QString prog = whichBin(str);
		if (!prog.isEmpty())
			return str;
	}
	return "";
}

bool corelib::forceFuseiso()
{
	return settings->value("ForceFuseiso", false).toBool();
}

void corelib::setForceFuseiso(bool value, bool isempty)
{
	if (whichBin("fuseiso").isEmpty())
		return;
	setConfigValue("ForceFuseiso", value, isempty);
}

void corelib::setDiscDir(QString dir, bool isempty)
{
	setConfigValue("DiscDir", dir, isempty);
}

QString corelib::discDir()
{
	return settings->value("DiscDir").toString();
}

QString corelib::config()
{
	if (QProcessEnvironment::systemEnvironment().contains("XDG_CONFIG_HOME"))
		return QProcessEnvironment::systemEnvironment().value("XDG_CONFIG_HOME") + QDir::separator() + "winegame.conf";
	else
		return QDir::homePath() + "/.config/winegame.conf";
}

bool corelib::removeDir(const QString & dir)
{
	QDir dirObj(dir);
	int i = 0;
	int max = dirObj.entryList(QDir::Readable | QDir::NoDotAndDotDot | QDir::AllEntries).count();
	ui->progressText(tr("Removing %1").arg(dir));
	foreach (QString fileName, dirObj.entryList(QDir::Readable | QDir::NoDotAndDotDot | QDir::AllEntries))
	{
		i++;
		ui->progressRange(i, max);
		if (QFileInfo(dir+QDir::separator()+fileName).isDir())
			removeDir(dir+QDir::separator()+fileName);
		else
			dirObj.remove(fileName);
	}
	if (!dirObj.rmdir(dir))
		return false;

	return true;
}

bool corelib::copyDir(const QString &dir, const QString &destination)
{
	copyCancelled = false;
	static bool isProgressBarShow = false;
	QDir myDir(dir);
	myDir.mkpath(destination);
	if (!isProgressBarShow)
	{
		ui->showProgressBar(tr("Copying %1 into %2").arg(dir).arg(destination));
		isProgressBarShow = true;
	}
	int max = myDir.entryList(QDir::NoDotAndDotDot).count();
	int i = 0;
	foreach (QString fileName, myDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries))
	{
		qApp->processEvents();
		if (copyCancelled)
		{
			if (isProgressBarShow)
				ui->endProgress();
			return false;
		}
		i++;
		ui->progressText(tr("Copying %1 into %2").arg(dir + QDir::separator() + fileName).arg(destination + QDir::separator() + fileName));
		ui->progressRange(i,max);
		if (QFileInfo(dir + QDir::separator() + fileName).isDir())
		{
			qDebug() << "copying " << dir + QDir::separator() + fileName << " into " << destination + QDir::separator() + fileName;
			copyDir (dir + QDir::separator() +fileName, destination + QDir::separator() + fileName);
		}
		else
		{
			QFile file (dir + QDir::separator() + fileName);
			file.copy(destination + QDir::separator() + fileName);
		}
	}
	if (isProgressBarShow)
		ui->endProgress();
	return true; //others not implemented yet;
}

void corelib::cancelCopy()
{
	copyCancelled = true;
}

QString corelib::shareDir() const
{
QString shareDir;
QDir dir (qApp->applicationDirPath());
if (dir.dirName() == "bin") //like a system directory
{
	dir.cdUp();
	dir.cd("share");
	if (!dir.exists("winegame"))
		return "";
	dir.cd("winegame");
	shareDir = dir.absolutePath();
}

}

void corelib::initDb()
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q (db);
   q.prepare("CREATE TABLE Apps (id INTEGER PRIMARY KEY, prefix TEXT, wineprefix TEXT, wine TEXT)");
	 if (!q.exec())
   {
   ui->error( tr("Database error"), tr("Failed to create table for storing installed applications. See errors on console"));
   qDebug() << "DB: Query error " << q.lastError().text();
   qApp->exit (-24);
	 }
	 q.prepare("CREATE TABLE Names (id INTEGER PRIMARY KEY, prefix TEXT, name TEXT, note TEXT, lang TEXT)");
	 if (!q.exec())
   {
   ui->error( tr("Database error"), tr("Failed to create table for storing installed applications. See errors on console"));
   qDebug() << "DB: Query error " << q.lastError().text();
   qApp->exit (-24);
	 }
 }

void corelib::setConfigValue(QString key, QVariant value, bool setIfEmpty)
{
	if (setIfEmpty)
	{
		if (settings->value(key).isNull()) //recursive call
			setConfigValue(key, value, false);
	}
	else
	{
		if (key == "PackageDir") //little hack
			QDir::setSearchPaths("packages", value.toStringList());
		else if (key == "WineDir")
			QDir::setSearchPaths("winedir", QStringList(value.toString()));
		settings->setValue(key, value);
	}
}

bool corelib::syncPackages()
{
	foreach (QString mirror, syncMirrors())
	{
		if (mirror.isEmpty())
			continue;
	// инициализирую QtNetwork классы
	//загружаю файл LAST
	QEventLoop loop;
	QNetworkAccessManager *manager = new QNetworkAccessManager (this);
	QNetworkRequest req; //request для Url
	req.setUrl(QUrl(mirror + "/LAST"));
	req.setRawHeader("User-Agent", "Winegame-Browser 0.1");
	QNetworkReply *reply = manager->get(req);
	currentReply = reply;
	connect (reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(setRange(qint64,qint64)));
	connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
	connect (reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT (error(QNetworkReply::NetworkError)));
	ui->showProgressBar(tr("Downloading winegame packagelist"), SLOT(cancelCurrentOperation()), this);
	ui->progressText(tr("Downloading winegame release info...."));
	loop.exec();
	ui->endProgress();
	QByteArray relInfo = reply->readAll();
	if (relInfo.isEmpty())
	{
		qDebug() << "wgpkg: failed to fetch packages from" << mirror << "URL " << req.url();
		return false;
	}
	//открываем ~/.winegame/packages/LAST
	QFile file (packageDirs().first() + "/LAST");
	if (!file.exists())
		file.open(QIODevice::WriteOnly);
	else
	{
		//читаем содержимое LAST, сравнивая его с relInfo
		file.open(QIODevice::ReadOnly);
		if (relInfo == file.readAll())
			return true;
		//закрываем файл и открываем его в режиме truncate
		file.close();
		file.open(QIODevice::WriteOnly | QIODevice::Truncate);
		}
	//записываем relInfo
	file.write(relInfo);
	file.close();
	//загружаю дистрибутив package-latest.tar.bz2
	req.setUrl(QUrl(mirror + "/packages-latest.tar.bz2"));
	QNetworkReply *reply2 = manager->get(req);
	currentReply = reply2;
	connect (reply2, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(setRange(qint64,qint64)));
	connect (reply2, SIGNAL(finished()), &loop, SLOT(quit()));
	connect (reply2, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT (error(QNetworkReply::NetworkError)));
	ui->showProgressBar(tr("Downloading winegame package index"), SLOT(cancelCurrentOperation()), this);
	ui->progressText(tr("Downloading winegame packages..."));
	loop.exec();
	ui->endProgress();
	//записываем в /tmp
	QString tfile = QDir::tempPath() + "/index.tar.bz2";
	file.setFileName(tfile);
	file.open(QIODevice::WriteOnly);
	file.write(reply2->readAll());
	file.close();
	bool res = unpackWine(tfile, packageDirs().first());
	file.remove();
	if (!res)
		return false;
}
	return true;
}

int corelib::runGenericProcess(QProcess *process, const QString &program, QString message)
{
 /*
   Запускает программу program в процессе process, при этом
   показывая прогрессбар winegame через UiClient
   */
	if (message.isEmpty())
		message = tr("The process is running");
	QEventLoop loop;
	ui->showUserWaitMessage(message);
	connect(process, SIGNAL(finished(int)), &loop, SLOT(quit()));
	process->start(program);
	loop.exec();
	ui->closeWaitMessage();
	return process->exitCode();
}

QStringList corelib::syncMirrors()
{
	//не используем механизм QStringList для QSettings, т.к. нельзя. (отпадает возможность удобного ручного редактирования
	return settings->value("hosts/sync").toString().split(";", QString::SkipEmptyParts);
}
void corelib::setSyncMirrors(QStringList urls, bool isempty)
{
	foreach (QString str, urls)
	{
		if (!QUrl(str).isValid())
			urls.removeOne(str);
	}
	if (urls.isEmpty())
		return;
	QString value  = urls.join(";");
	setConfigValue("hosts/sync", value, isempty);
}
/*!
 Отменяет текущую операцию загрузки (см. corelib::downloadWine())
  */
void corelib::cancelCurrentOperation()
{
	if (currentReply)
		currentReply->abort();
	else
		qDebug() << "WARNING: access to null pointer";
}
