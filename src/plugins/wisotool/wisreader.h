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


#ifndef WISREADER_H
#define WISREADER_H
#include "sourcereader.h"

struct WisObject
{
	QString id;
	QString description;
	//hmm, nothing for now so far
};


class WisReader : public SourceReader
{
    Q_OBJECT
public:
	explicit WisReader(QObject *parent, corelib *lib, WisObject label);
	bool checkWine () {return true;}
	bool isMulticd () {return false;}
	short int discCount() {return 0;}
	QString name();
	QString note();
	QString realName () {return name();}
	QString realNote () {return note();}
	bool setup(const QString &file);
	bool needFile() {return false;}
	void  setDvd (const QString &device, const QString &path);
	Prefix * prefix ();
	QString prefixPath ();
	QString wine () {core->whichBin("wine");}
	QStringList components () {}
	QString icon () {return  "";}
	bool detectApp (QString path);
	QString defaultWine () {return wine();}
	bool needToSetMemory () {return true;}
	Name nameForLang (QString locale);
	QStringList locales() {return QStringList("C");} //We support only C locale

protected:
	QString desc;

};

#endif // WISREADER_H
