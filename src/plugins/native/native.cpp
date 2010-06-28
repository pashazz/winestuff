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


#include "native.h"

NativeFormat::NativeFormat()
{
}

QList <SourceReader *> NativeFormat::readers(corelib *core, bool includeDvd)
{
	Q_UNUSED(includeDvd);
	QStringList dirlist = core->packageDirs();
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

bool NativeFormat::updateAllWines(corelib *core)
{
	QStringList dirlist = core->packageDirs();
	foreach (QString dir, dirlist)
	{
		foreach (QString pkg, QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot))
		{
			NativeReader *reader = new NativeReader(pkg, core);
			if (reader->ID().isEmpty())
				continue;
			if (!reader->checkWine())
				return false;
		}
	}
	return true;
}
