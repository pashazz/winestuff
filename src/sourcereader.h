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

#ifndef SOURCEREADER
#define SOURCEREADE
#include "prefix.h"

class SourceReader : public QObject
{
	Q_OBJECT
public:
	explicit SourceReader(QString prefixId, corelib *lib) :id(prefixId), core(lib) {}
	virtual ~SourceReader(); {}
	virtual static QStringList prefixes (const QString &directory) = 0;
	virtual bool checkWine (QString prefixId) = 0;
	virtual QString name() = 0;
	virtual QString note() = 0;
	virtual QString setup() = 0;
	virtual QString prefixPath () = 0;
	virtual QString wine () = 0;
	virtual QStringList components () = 0;
	virtual QString filesDirectory () = 0;
	virtual QString icon () = 0;
	virtual QString preinstCommand () = 0;
	virtual QString postinstCommand () = 0;

protected:
	QString id;
	corelib *core;
};

#endif //SOURCEREADER
