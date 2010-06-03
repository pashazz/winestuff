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

#include "poldownloader.h"

PolDownloader::PolDownloader(Prefix *prefixObj)
	:QObject(prefixObj), prefix(prefixObj), URL("http://wine.playonlinux.com/linux-i386/")
{
	core = prefix->myLib();
	goodGet = true; //обозначает, успешна ли загрузка списка.
	QEventLoop loop;
	QNetworkAccessManager *manager = new QNetworkAccessManager (this);
	QNetworkRequest req; //request для Url
	req.setUrl(QUrl(URL+"LIST"));
	req.setRawHeader("User-Agent", "Winegame-Browser 0.1");
	QNetworkReply *reply = manager->get(req);
	core->client()->showProgressBar(tr("Downloading a list of wines"));
	core->client()->progressText(URL);
	connect (reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(setProgressRange(qint64,qint64)));
	connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
	connect (reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT (error(QNetworkReply::NetworkError)));
	//получаем LIST
	loop.exec();
	core->client()->endProgress();
	if (!goodGet)
		return;
	qDebug() << "got a wines list";
	QTextStream stream (reply);
	while (!stream.atEnd())
	{
		QString str = stream.readLine();
		QStringList myList = str.split(";", QString::SkipEmptyParts);
		if (myList.length() < 3)
		{
			qDebug() << "PolDownloader: warning: wrong string";
			continue;
		}
		files <<  myList.at(0);
		versions << myList.at(1);
		sha1sums << myList.at(2);
	}
}

void PolDownloader::error(QNetworkReply::NetworkError error)
{
	if (error != QNetworkReply::NoError)
	goodGet = false;
}

void PolDownloader::setProgressRange (qint64 got, qint64 total)
{
	int kbGot = got/1024;
	int kbTotal=total/1024;
	core->client()->progressRange(kbGot, kbTotal);
}

bool PolDownloader::checkSHA1(QString file)
{
	if (!goodGet)
		return false;
	/* GUI freezes at this point */
	QProcess *p = new QProcess (this);
	p->start(core->whichBin("sha1sum") + " " + file);
	p->waitForFinished(-1);
	if (p->exitCode() != 0)
		return false;
	QString mySum = p->readAllStandardOutput().trimmed();

	QString name = QFileInfo (file).fileName();
	if (name.isEmpty())
		return false;
	QString realSum = sha1sums.at(files.indexOf(name));
	qDebug() << "PolDownloader-SHA1: file sum is " << mySum << " real sum is " << realSum;
	return mySum != realSum;

}

bool PolDownloader::downloadWine(QString version)
{
//Проверяем, есть ли уже данный вайн
QFile file (core->wineDir() + "/wineversion/" + version + "/usr/bin/wine");
if (file.exists())
{
	qDebug() << " Wine downloaded, skipping....";
	return true;
}
QString wineFileName = files.at(versions.indexOf(version));
if (wineFileName.isEmpty())
	return false;
QString archive = core->downloadWine(URL + wineFileName);
if (archive.isEmpty())
	return false;
if (!checkSHA1(archive))
	return false; //experimental
if(!core->unpackWine(archive, core->wineDir()))
	return false;

return true;
}

bool PolDownloader::setWineVersion(QString version)
{
	//trying to download wine
	if (!downloadWine(version))
		return false;
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q (db);
	q.prepare("UPDATE Apps SET wine=:wine WHERE prefix=:prefix");
	q.bindValue(":wine", core->wineDir() + "/wineversion/" + version + "/usr/bin/wine");
	q.bindValue(":prefix", prefix->prefixName());
	if (!q.exec())
	{
		qDebug() << "PolDownloader: failed to execute SQL query " << q.lastError().text();
		return false;
	}
	return true;
}
void PolDownloader::fallback()
{
	//back to app`s wine
	QString wine;
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q (db);
	q.prepare("UPDATE Apps SET wine=:wine WHERE prefix=:prefix");
	if (prefix->distr().isEmpty()) //like a system wine
		wine = core->whichBin("wine");
	else //like a custom wine
		wine = core->wineDir() + "/wines/" + prefix->prefixName() + "/usr/bin/wine";
	q.bindValue(":wine", wine);
	q.bindValue(":prefix", prefix->prefixName());
	if (!q.exec())
	{
		qDebug() << "PolDownloader: failed to execute SQL query " << q.lastError().text();
	}

}

QString PolDownloader::detectCurrentVersion()
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery q (db);
	q.prepare("SELECT wine FROM Apps WHERE prefix=:prefix");
	q.bindValue(":prefix", prefix->prefixName());
	if (!q.exec())
	{
		qDebug() << "PolDownloader: failed to execute SQL query " << q.lastError().text();
	}
	q.first();
	QString wine = q.value(0).toString();
	//Analyze this. Try to detect version number
	QStringList myList(versions);
	qSort(myList.begin(), myList.end(), qGreater<QString>()); //новые версии wine- раньше.
	foreach (QString str, myList)
	{
		if (wine.contains(str))
			return str;
	}
	return "";
}
