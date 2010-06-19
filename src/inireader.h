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


#ifndef INIREADER_H
#define INIREADER_H
//#include "sourcereader.h"
#include "corelib.h"

typedef QPair <QString, QString> Name;

class WINESTUFFSHARED_EXPORT SourceReader : public QObject
{
public:
	Q_OBJECT
public:
	explicit SourceReader(QString confId, corelib *lib, QObject *parent = 0) :id(confId), core(lib),  QObject (parent),s(new QSettings("packages:" + id + "/control", QSettings::IniFormat, parent)) {}
	 ~SourceReader() {}
	 static QStringList configurations (const QStringList &directories); //возвращает ДОСТУПНЫЕ конфигурации, т.е. те, в которые приложения НЕ БЫЛИ установлены.
	 static bool updateWines (const QStringList &prefixes, corelib *core);
	 bool checkWine ();
	QString ID () {return id;}
	 QString name();
	 QString note();
	 QString setup();
	 QString prefixPath ();
	 QString wine ();
	 QStringList components ();
	 QString filesDirectory ();
	 QString icon ();
	 QString preinstCommand ();
	 QString postinstCommand ();
	 QStringList availableDiscs();
	 QStringList discFileList (const QString &disc);
	 static QString defaultWine (const QString &id); // default wine for prefix ID in this implementation
	 bool needToSetMemory ();
	 Name nameForLang (QString locale);
	 QStringList locales();
	 bool isMulticd ();
	 short int discCount();
signals:
	void presetPrefixNeed(QString &prefix);
	void presetNameNeed (QString &name);
	void presetNoteNeed (QString &note);
protected:
	QString id;
	corelib *core;
	QSettings *s;
	static bool isPrefixInstalled(QString prefixName, QSqlDatabase db = QSqlDatabase::database());
private slots:

private:
	QString _name;
	QString _note;
	QString _prefix;

	bool downloadWine();
	QString distr();
	void writeMD5(const QString &md5sum);
	QString getMD5();

	inline QString workdir();
};

#endif
