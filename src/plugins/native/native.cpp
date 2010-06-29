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

SourceReader *  NativeFormat::readerById(const QString &id, corelib *core)
{
	foreach (SourceReader *reader, readers(core, true))
	{
		if (reader->ID() == id)
			return reader;
	}
	return 0;
}

Q_EXPORT_PLUGIN2(wst_native, NativeFormat)
