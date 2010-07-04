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

Prefix::Prefix (QObject *parent, corelib *lib)
	:QObject(parent), core(lib), id(""), _path("") {}

Prefix::Prefix (const QString &id, const QString &name, const QString &note, const QString &path, const QString &wine, QObject *parent, corelib *lib)
	:QObject (parent), core(lib)
{
	setID(id);
	setName(name);
	setNote(note);
	setPath(path);
	setWine(wine);
}

void Prefix::setMemory()
{
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
	p.setProcessEnvironment(environment());
	core->runGenericProcess(&p, QString ("%1 regedit %2").arg(wine()).arg(f.fileName()), tr("Updating video memory, application %1").arg(name()));
	f.remove();
}

void Prefix::makeDesktopIcon(const QString &name, const QString &program, const QString &icon)
{
	QFile file  (core->client()->desktopLocation() + QDir::separator() + name + ".desktop");
	qDebug() << "Making desktop icon" << file.fileName();
	QTextStream str (&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		qDebug() << "Prefix: unable to open file" << file.fileName();
		return;
	}
	str << "[Desktop Entry]\n";
	str << QString ("Name=%1\n").arg(name);
	str << "Type=Application\n";
	str << QString("Exec=env WINEPREFIX=\"%1\" winegame -r \"%2\"\n").arg(_path).arg(program);
	if (!icon.isEmpty())
		str << QString ("Icon=%1\n").arg(icon);
	str << QString ("Comment=%1\n").arg(_note);
	str << QString("Categories=%1\n").arg("Game;"); //temporalily,
	file.close();
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

int Prefix::runApplication(const QString &program, QString workingDirectory, bool block)
{
	if (program.isEmpty())
		return -100; //программа не указана
	if (_wine.isEmpty())
		_wine = corelib::whichBin("wine");
	if (workingDirectory.isEmpty())
		workingDirectory = QFileInfo(program).absolutePath();
	QProcess *p = new QProcess (this);
	p->setProcessEnvironment(environment());
	p->setWorkingDirectory(workingDirectory);
	QEventLoop loop;
	if (block)
	{ //блокируем действия юзера в программе модальным диалогом.
		core->client()->showUserWaitMessage(tr("Running program %1, please wait").arg(QFileInfo(program).fileName()));
	}
	connect (p, SIGNAL(finished(int)), &loop, SLOT(quit()));
	p->start(_wine, program.split(" ", QString::SkipEmptyParts));
	loop.exec();
	if (block)
		core->client()->closeWaitMessage();
	return p->exitCode();
	}

QProcessEnvironment  Prefix::environment ()
{
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("WINEPREFIX", _path);
	env.insert("WINE", _wine);
	if (!_diskroot.isEmpty())
		env.insert("CDROOT", _diskroot);
	return env;
	}

void Prefix::launch_c()
{
	QProcess p;
	QEventLoop loop;
	connect (&p, SIGNAL(finished(int)), &loop, SLOT(quit()));
	QDir c (_path + "/dosdevices/c:");
	p.setWorkingDirectory(c.path());
	p.setProcessEnvironment(environment());
	p.start("xdg-open", QStringList(c.path()));
	loop.exec();
}
