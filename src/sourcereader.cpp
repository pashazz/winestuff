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

#include "sourcereader.h"

SourceReader::SourceReader(QObject *parent, corelib *core, QString id) :
    QObject(parent)
{
	this->core = core;
	this->id = id;
}

QString SourceReader::executable(const QString &file)
{
	if (file.endsWith(".exe", Qt::CaseInsensitive))
		return file;
	else if (file.endsWith(".msi", Qt::CaseInsensitive))
		return QString ("msiexec \"%1\"").arg(file);
	else if (file.endsWith(".bat", Qt::CaseInsensitive))
		return QString("wineconsole.exe \"%1\"").arg(file);
	else /* Oh my god */
	{
		qDebug() << "WARNING: Incompatible file";
		return file;
	}
}

bool SourceReader::isIdValid(const QString &id)
{
	QStringList invSymbols = QStringList () << ";"  << "wines" << "wineversion" << QDir::separator() << " ";
	foreach (QString str, invSymbols)
	{
		if (id.contains(str, Qt::CaseInsensitive))
			return false;
	}
	return true;
}
