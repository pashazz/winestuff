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
