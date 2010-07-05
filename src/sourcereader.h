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


#ifndef SOURCEREADER_H
#define SOURCEREADER_H

#include <QObject>
#include "corelib.h"
#include "prefix.h"
typedef QPair <QString, QString> Name;

class SourceReader : public QObject
{
	Q_OBJECT
public:
	explicit SourceReader(QObject *parent, corelib *core, QString id);
	virtual ~SourceReader() {}
	virtual bool checkWine () = 0;
	virtual bool isMulticd () = 0;
	virtual short int discCount() = 0;
	QString ID () {return id;}
	virtual QString name() = 0;
	virtual QString note() = 0;
	virtual QString realName () = 0;
	virtual QString realNote () = 0;
	virtual bool setup(const QString &file) = 0;
	virtual bool needFile () = 0;
	virtual void  setDvd (const QString &device, const QString &path) = 0;
	virtual Prefix * prefix () = 0;
	virtual QString prefixPath () = 0;
	virtual QString wine () = 0;
	virtual QStringList components () = 0;
	virtual QString icon () = 0;
	virtual bool detectApp (QString path) = 0;
	virtual QString defaultWine () = 0;
	virtual bool needToSetMemory () = 0;
	virtual Name nameForLang (QString locale) = 0;
	virtual QStringList locales() = 0;
protected:
	QString id;
	corelib *core;
	QString executable (const QString &file);
	static bool isIdValid(const QString &id);
};

#endif // SOURCEREADER_H
