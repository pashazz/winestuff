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

#include "dvdrunner.h"
#include <QtDebug>
DVDRunner::DVDRunner(corelib *lib, QString path, PluginWorker *worker)
	:QObject (0),core (lib), mounted (false), cancelled (false), _worker(worker)
{
	//Пробуем определить тип
	QFileInfo info (path);
	if (info.isDir())
	{
		//Внимание, директории с исошками не поддерживаются (пока)
		diskPath = path;
		type = Pashazz::Real;
	}
	else if (info.isFile())
	{
		realDrive = path;
		diskPath = core->mountDir();
		//Пробуем примонтировать образ в diskPath
		if (core->getSudoProg().isEmpty() || core->forceFuseiso()) {
		mount = QString("fuseiso \"%1\" \"%2\"").arg(realDrive).arg(diskPath);
		umount = QString("fusermount -u \"%1\"").arg(diskPath);
	}
		else
		{
			QString sudo = core->getSudoProg();
			QString mountString = QString ("mount -o loop \"%1\" \"%2\"").arg(realDrive).arg(diskPath);
			QString umountString = QString ("umount \"%1\"").arg(diskPath);
			if (sudo == "kdesu" && QProcessEnvironment::systemEnvironment().contains("KDE_FULL_SESSION"))
			{
				mount = QString ("kdesu -i %1 \"%2\"").arg("winegame").arg(mountString);
				umount = QString ("kdesu -i %1 \"%2\"").arg("winegame").arg(umountString);
			}
			else if (sudo == "gksu")
			{
			 QString mountMsg = tr("Enter password to mount ISO image");
			 QString umountMsg = tr("Enter password to unmount ISO image");
			 mount = QString ("gksu -m \"%1\" -D %2 \"%3\"").arg(mountMsg).arg("WineStuff").arg(mountString);
			 umount = QString ("gksu -m \"%1\" -D %2 \"%3\"").arg(umountMsg).arg("WineStuff").arg(umountString);
				  }
			else if (sudo == "xdg-su")
			{
				mount = QString ("xdg-su -c \"%1\"").arg(mountString);
				umount = QString ("xdg-su -c \"%1\"").arg(umountString);
			}
			else
			{
				//force fuseiso
				mount = QString("fuseiso \"%1\" \"%2\"").arg(realDrive).arg(diskPath);
				umount = QString("fusermount -u \"%1\"").arg(diskPath);
			}
		}
		type = Pashazz::Image;
	}
	else
	{
		type = Pashazz::Unknown;
		core->client()->error(tr("Execution error"), tr("I/O error"));
	}
	result =	prepare();
}

bool DVDRunner::prepare(bool nodetect)
{
	if (type == Pashazz::Unknown)
		return false;
	//1) монтируем образ, если это образ.
	if (type == Pashazz::Image  && (!mounted))
	{
		QProcess p (this);
		qDebug() << mount;
		p.start(mount);
		p.waitForFinished(-1);
		if (p.exitCode() != 0)
		{
			mounted = false;
			return false;
		}
		else
			mounted = true;
	}

	//2) детектинг. Если юзер сам указал префикс, тогда аргумент nodetect должен быть true. Иначе detect попытается найти нужный пакет и создать объект Prefix
	if (!nodetect)
	{
		qDebug() << "Detecting disc....";
		if (!detect ())
		{
			qDebug() << "Unable to detect disc";
			return false;
		}
}
	//3)Проверяем, возможно наша игра на нескольких дисках

	if (reader->isMulticd ())
	{
		qDebug() << "Multidisc detected";
		for (int i=1; i <= reader->discCount(); i++)
		{
			if (i != 1)
			{
				bool result = false;
				insertnextcd:
				core->client()->insertNextCd(result, QVariant(i).toString());
				if (!result)
				{
					qDebug () << "Exiting (by user).....";
					break;
				}
				else
				{
					if (!checkDisc(diskPath))
						goto insertnextcd;
				}
			}
			entrylist = QDir(diskPath).entryList();
			if (!core->copyDir(diskPath, core->discDir()))
			{
				qDebug() << "Unable to copy directory..." << diskPath;
				core->client()->endProgress();
				return false;
			}
		}
		diskPath = core->discDir();
	}
	//вроде все.
	return true;
}

bool DVDRunner::checkDisc(QString &diskPath) //проверяет диск. Если пусто, спрашивает у пользователя новую директорию, поэтому diskPath передается по ссылке.
{
	QDir dpath (diskPath);
	checkdisc:
	if ((dpath.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0) || (entrylist == dpath.entryList()))
	{
		// Спрашиваем у клиента директорию.
		diskPath = core->client()->directoryDialog(tr("Select disc directory...."), dpath.cleanPath(dpath.path() + "/.."));
		if (diskPath.isEmpty())
		{ //Возвращаем значение diskPath
			diskPath = dpath.absolutePath();
			return false;
		}
		else
		{//проверяем значение
			dpath.setPath(diskPath);
			goto checkdisc;
		}
	}
	else
		return true;
}


void DVDRunner::setReader(SourceReader *reader)
{
	this->reader = reader;
	prepare(true); //true - отключаем детектинг (префикс установлен вручную)
}

bool DVDRunner::detect()
{
	foreach (FormatInterface *interface, _worker->plugins())
	{
		foreach (SourceReader *reader, interface->readers(true))
		{
			if (reader->detectApp(diskDirectory()))
			{
				this->reader = reader;
				return true;
			}
			else //Just delete it
				delete reader;
		}
		return false;
	}
}


void DVDRunner::cleanup()
{
	//размонтируем наш сидюк
	if (type == Pashazz::Image)
	{
		if (!cancelled)
			core->client()->infoDialog(tr("Information"), tr("Press OK/Enter when application`s installation ends"));
		QProcess p (this);
		p.start(umount);
		p.waitForFinished(-1);
		mounted = false;
	}
	core->client()->showProgressBar("Cleaning up....");
	core->removeDir(core->discDir());
	core->client()->endProgress();
}

QString DVDRunner::exe ()
{
	QString exe;
	//Теперь просмотрим AutoRun
	if (!core->autorun(diskPath).isEmpty())
	{
		QSettings autorun(core->autorun(diskPath), QSettings::IniFormat, this);
		autorun.beginGroup("autorun");
		if (!autorun.value("open").toString().isEmpty())
		{
			exe = diskPath + QDir::separator() + autorun.value("open").toString();
			return exe;
		}
	}
	//А теперь спросим EXE у пользователя.
	core->client()->selectExe(tr("Select EXE file"), exe, diskPath);
	return exe;
}

QString DVDRunner::diskDirectory()
{
	if (realDrive.isEmpty() || diskPath != core->mountDir())
		return diskPath;
	else
		return realDrive;
}

void DVDRunner::cancel()
{
	cancelled = true;
	cleanup();
}
