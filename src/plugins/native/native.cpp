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


#include "native.h"

NativeFormat::NativeFormat()
{
	mirrorFile = "native.mirrors";
	dirsFile = "native.dirs";
}

void NativeFormat::on_loadPlugin(corelib *lib)
{
	core = lib;
	_coreDirectory = core->configPath();
	checkFiles();
}

QList <SourceReader *> NativeFormat::readers(bool includeDvd)
{
	Q_UNUSED(includeDvd);
	QStringList dirlist = packageDirs();
	QList <SourceReader *> readers;
	foreach (QString dir, dirlist)
	{
		foreach (QString pkg, QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		{
			SourceReader *reader = new NativeReader(pkg, core);
			if (!reader->ID().isEmpty())
				readers.append(reader);
			else
				qDebug() << "wst-native: unable to retreive Reader from " << pkg;
		}
	}
	return readers;
}

bool NativeFormat::hasFeature(Pashazz::Feautures feature)
{
	switch (feature)
	{
	case Pashazz::Detecting:
			return true;
	case Pashazz::Multidisc:
			return true;
	case Pashazz::NoFeatures:
			return false;
	}
}

bool NativeFormat::updateAllWines(PrefixCollection *collection)
{
	//check packages
	int i = 0;
	foreach (QUrl mirror, syncMirrors())
	{
		if (mirror.isEmpty())
			continue;
		// инициализирую QtNetwork классы
		//загружаю файл LAST
		QEventLoop loop;
		QNetworkAccessManager *manager = new QNetworkAccessManager (this);
		QNetworkRequest req; //request для Url
		req.setUrl(QUrl(mirror.toString() + "/LAST"));
		req.setRawHeader("User-Agent", "Winegame-Browser 0.1");
		QNetworkReply *reply = manager->get(req);
		connect (reply, SIGNAL(finished()), &loop, SLOT(quit()));
		core->client()->showUserWaitMessage(tr("Updating Packages..."));
		loop.exec();
		core->client()->closeWaitMessage();
		QByteArray relInfo = reply->readAll();
		if (relInfo.isEmpty())
		{
			qDebug() << "wgpkg: failed to fetch packages from" << mirror << "URL " << req.url();
			return false;
		}
		//открываем ~/.winegame/packages/LAST
		QFile file (packageDirs().at(i) + "/LAST");
		if (!file.exists())
			file.open(QIODevice::WriteOnly);
		else
		{
			//читаем содержимое LAST, сравнивая его с relInfo
			file.open(QIODevice::ReadOnly);
			if (relInfo == file.readAll())
			{
				goto checkwines;
				file.close();
			}
			//закрываем файл и открываем его в режиме truncate
			file.close();
			file.open(QIODevice::WriteOnly | QIODevice::Truncate);
		}
		//записываем relInfo
		file.write(relInfo);
		file.close();
		//загружаю дистрибутив package-latest.tar.bz2

		QString tfile = core->downloadWine(mirror.toString() + "/packages-latest.tar.bz2", true);
		if (!QFile::exists(tfile))
			return false;
		bool res = core->unpackWine(tfile, packageDirs().at(i));
		if (!res)
			return false;
		i++;
	}

		checkwines:
	//Check wines.
	QStringList dirlist = packageDirs();
	foreach (QString dir, dirlist)
	{
		foreach (QString pkg, QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		{
			NativeReader *reader = new NativeReader(pkg, core);
			if (reader->ID().isEmpty())
				continue;
			if (collection->havePrefix(reader->ID()))
			{
				if (!reader->checkWine())
				return false;
			}
		}
	}
	return true;
}

SourceReader *  NativeFormat::readerById(const QString &id)
{
	foreach (SourceReader *reader, readers(true))
	{
		if (reader->ID() == id)
			return reader;
	}
	return 0;
}

void NativeFormat::checkFiles()
{
	if (_coreDirectory.isEmpty())
		return;
	QTextStream stream;
	QFile mirrorfile (_coreDirectory + QDir::separator() + mirrorFile);
	stream.setDevice(&mirrorfile);
	if (mirrorfile.exists())
	{
		int i = 0;
		//try to reaad all the file, heh
		mirrorfile.open(QIODevice::ReadOnly | QIODevice::Text);
		//ignore strings starts with '#'
		while (!stream.atEnd())
		{
			QString str = stream.readLine().trimmed();
			if ((!str.isEmpty()) || (!str.startsWith('#')))
				i++;
		}
		mirrorfile.close();
		if (i == 0)
			goto dontexist;
	}
	else
	{
		dontexist:
		mirrorfile.open(QIODevice::WriteOnly | QIODevice::Text);
		mirrorfile.write("http://winegame-project.ru/winepkg");
		mirrorfile.close();
	}
	//do the same for dirfile
	mirrorfile.setFileName(_coreDirectory + QDir::separator() + dirsFile);
	stream.setDevice(&mirrorfile);
	if (mirrorfile.exists())
	{
		int i = 0;
		//try to reaad all the file, heh
		mirrorfile.open(QIODevice::ReadOnly | QIODevice::Text);
		//ignore strings starts with '#'
		while (!stream.atEnd())
		{
			QString str = stream.readLine().trimmed();
			if ((!str.isEmpty()) || (!str.startsWith('#')))
				i++;
		}
		mirrorfile.close();
		if (i == 0)
			goto dontexist2;

	}
	else
	{
		dontexist2:
		mirrorfile.open(QIODevice::WriteOnly | QIODevice::Text);
		stream << QString("%1/packages").arg(_coreDirectory);
		mirrorfile.close();
	}
}

QStringList NativeFormat::packageDirs()
{
	QFile file (_coreDirectory + QDir::separator() + dirsFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return QStringList();
	//Eixed.  Read file
	QTextStream stream (&file);
	QStringList list;
	while (!stream.atEnd())
	{
		QString path = stream.readLine().trimmed();
		if (path.isEmpty() || path.startsWith('#'))
			continue;

		QDir dir (path);
		if (!dir.exists())
		{
			qDebug() <<"native: directory " << path << "do not exist. Trying to create....";
			if (!dir.mkpath(dir.path()))
			{
				qDebug() << "native: failed to create directory" << path;
				continue;
			}
		}
		list << path;
	}
	if (QDir::searchPaths("nativepackages") != list)
		QDir::setSearchPaths("nativepackages", list);
	return list;
}

QUrlList NativeFormat::syncMirrors()
{
	QFile file (_coreDirectory + QDir::separator() + mirrorFile);
	if ((!file.exists()) || (!file.open(QIODevice::ReadOnly | QIODevice::Text)))
	{
		return QUrlList();
	}
	QTextStream stream (&file);
	QUrlList list;
	while (!stream.atEnd())
	{
		QString urlpath = stream.readLine().trimmed();
		if (urlpath.isEmpty() || urlpath.startsWith('#'))
			continue;
		QUrl url (urlpath);
		if (url.isValid())
			list << url;
		else
			qDebug() << "native: " << "Url " << urlpath << "is not valid...";
	}
	return list;
}

Q_EXPORT_PLUGIN2(wst_native, NativeFormat)
