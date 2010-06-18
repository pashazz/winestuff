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


#ifndef PREFIXCOLL
#define PREFIXCOLL
#include "prefix.h"
#include "inireader.h"

class PrefixCollection : public QObject
{
	Q_OBJECT
public:
	PrefixCollection (QSqlDatabase database, corelib *lib, QObject *parent = 0);
	virtual ~PrefixCollection();
	Prefix* install (SourceReader *reader, QString file);
	bool remove (QString id);
	QList<Prefix*> prefixes ();
	Prefix* getPrefix (QString id);
	void setLocale (QLocale locale) {loc = locale;}
	QSqlDatabase database () {return db;}
	void setDatabase (QSqlDatabase database) {db = database;}
	void launchWinetricks (Prefix *prefix, const QStringList &args);
	bool updatePrefix (Prefix *newData, QString id = ""); //если id пустой, то используется newData.id(). Указывайте QString id, только если он изменен.
	bool havePrefix (const QString &id);
protected:
	static QString executable (QString file);

	/*Functions for Prefix */
	Name getName (QString locale, QString ID);

private:
	QSqlDatabase db;
	corelib *core;
	QLocale loc;
};

#endif // PREFIX_H
