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


#include "wisreader.h"

WisReader::WisReader(QObject *parent, corelib *lib, WisObject label) :
	SourceReader(parent, lib, label.id), desc(label.description)
{
}

void WisReader::setDvd(const QString &device, const QString &path)
{
	Q_UNUSED(device);
	Q_UNUSED(path);
	return;
}

bool WisReader::detectApp(QString path)
{
	Q_UNUSED(path);
	return false; //There is no detecting features
}

Name WisReader::nameForLang(QString locale)
{
	Q_UNUSED(locale);
	Name name;
	name.first = this->name();
	name.second = this->note();
	return name;
}
WisObject WisReader::getTrix(const QString &str)
{
	QStringList rawData = str.trimmed().split(" ", QString::SkipEmptyParts);
	WisObject trix;
	if (rawData.count() < 2)
		return trix;
	trix.id = rawData.takeFirst();
	trix.description = rawData.join(" ");
	if (trix.id != "help")
		return trix;
}
