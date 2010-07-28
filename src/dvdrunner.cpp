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
		updateMount();
	}
	else
	{
		type = Pashazz::Unknown;
		core->client()->error(tr("Execution error"), tr("I/O error"));
	}

	result =	prepare();
}
void DVDRunner::updateMount()
{
	realDrive.replace (" ", "\\ ");
	diskPath.replace (" ", " \\");
	//Пробуем примонтировать образ в diskPath
	if (core->getSudoProg().isEmpty() || core->forceFuseiso())
	{
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
	diskPath.replace ("\\ ", " ");
	realDrive.replace ("\\ ", " ");
}

bool DVDRunner::prepare(bool nodetect)
{
	if (type == Pashazz::Unknown)
		return false;
	//1) монтируем образ, если это образ.
	if (type == Pashazz::Image  && (!mounted))
	{
		QProcess p (this);
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
	else if (type == Pashazz::Real && (!mounted)) //Как бы монтирует система, поэтому мы только выставляем mounted в true
		mounted = true; //Собственно
	//2) детектинг. Если юзер сам указал префикс, тогда аргумент nodetect должен быть true. Иначе detect попытается найти нужный пакет и создать объект Prefix
	if (!nodetect)
	{
		if (!detect ())
			return false;
}

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
			if (reader->detectApp(diskPath))
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
		QProcess p (this);
		p.start(umount);
		p.waitForFinished(-1);
		if (p.exitStatus() == QProcess::NormalExit)
			mounted = false;
	}
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
	return exe;
}

QString DVDRunner::diskDirectory()
{
	return diskPath;
}

void DVDRunner::cancel()
{
	cancelled = true;
	cleanup();
}

void DVDRunner::eject(bool &ok)
{
	QStringList entrylist = QDir(diskPath).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	if (!reader)
	{
		core->client()->error(tr("Not implemented"), tr("This feature isn`t available yet for this configuration."));
		return;
	}
	Prefix *prefix = reader->prefix(); //нужный префикс

	//run Eject process
	QProcess p;
	p.setProcessEnvironment(prefix->environment());
	p.start(prefix->wine(), QStringList("eject"));
	p.waitForFinished(-1);
	if (p.exitCode() != 0)
		qDebug() <<  "DEBUG: UNABLE TO EJECT: " << p.readAllStandardError();

	//wait
	if (type == Pashazz::Real)
	{
		core->client()->infoDialog(tr("Insert disc"), tr("Insert disc and press Enter/OK. Don`t forget to mount it. If you need to use disk image or custom location of files, then just press Enter/OK.")); //и теперь после этого опрашиваем diskPath
		QStringList myList = QDir(diskPath).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
		if (myList != entrylist && (!myList.isEmpty()))
		{
			ok = true;
			return; //все, на этом мы закончили, diskPath изменять не надо.
		}
	}
	//спрашиваем у клиента точнку монт. или iso
	QString path;
	bool isDir = false;
	QString dir;
	if (type == Pashazz::Image)
		dir = QFileInfo (realDrive).absolutePath();
	core->client()->selectNextDisc(isDir, path, dir);
	if (path.isEmpty())
	{
		ok = false;
		return;
	}
	else
		cleanup();
	if (type == Pashazz::Image && mounted) //что-то тутявно не так, видимо юзер отмену нажал, нехорошо
	{
		ok = false;
		return;
	}
	if (isDir)
	{
		diskPath = path;
		prefix->setDiscAttributes(diskPath);
		type = Pashazz::Real;
	}
	else
	{
		//монтирую образ
		diskPath = core->mountDir();
		realDrive = path;
		prefix->setDiscAttributes(diskPath, realDrive);
		updateMount();
		QProcess p (this);
		if (core->runGenericProcess(&p, mount, tr("Mounting image")) != 0)
		{
			ok = false;
			return;
		}
		else
			mounted = true;
	}
	ok = true;
	core->client()->showNotify(tr("Disk is switched"), tr("Disk is switched to %1").arg(path));
}
