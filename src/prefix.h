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


#ifndef PREFIX_H
#define PREFIX_H
#include <QtCore>
#include "corelib.h"

class PrefixCollection;

class WINESTUFFSHARED_EXPORT Prefix : public QObject
{
	Q_OBJECT
friend class PrefixCollection; //for Prefix`s protected functions.
public:
	Prefix (QObject *parent, corelib *lib);
	Prefix (const QString &id,  const QString &name, const QString &note,  const QString &path, const QString &wine, QObject *parent, corelib *lib);
	virtual ~Prefix () {}
	/* Setters */
	void setName (const QString &name) {_name = name;}
	void setNote (const QString &note) {_note = note;}
	void setID (const QString &id) {this->id = id;}
	void setPath (const QString &path) {_path = path;}
	void setWine (const QString &wine) {_wine = wine;}
	void setDiscAttributes (QString diskRoot, QString imageFile = "/dev/cdrom")
	{
		_diskroot = diskRoot;
		_imagefile = imageFile;
		makeWineCdrom (diskRoot, imageFile);
	}

	/*Getters */
	QString name () {return _name;}
	QString note () {return _note;}
	QString ID () {return this->id;}
	QString path () {return _path;}
	QString wine() {return _wine;}
	QString diskRoot () {return _diskroot;}
	corelib* lib () {return core;}

	/* Process env. */
	QProcessEnvironment environment ();
	/* Other functions */
	int runApplication (const QString &program, QString workingDirectory = "");
	void makeDesktopIcon (const QString &name, const QString &program, const QString &icon);
	void setMemory ();
protected:
	/* Common functions */
	void makefix();
	void makeWineCdrom(const QString &path, const QString &device);

private:
	QString id;
	QString _name;
	QString _note;
	QString _path;
	QString _wine;
	QString _diskroot;
	QString _imagefile;
	corelib *core;
};



#endif // PREFIX_H
