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


#ifndef INIREADER_H
#define INIREADER_H
#include "sourcereader.h"
#include "corelib.h"
#include "prefix.h"

typedef QPair <QString, QString> Name;

class WINESTUFFSHARED_EXPORT NativeReader : public SourceReader
{
public:
	Q_OBJECT
public:
	explicit NativeReader(QString confId, corelib *lib, QObject *parent = 0) : SourceReader (parent, lib, confId), s(new QSettings("nativepackages:" + id + "/control", QSettings::IniFormat, parent)) {}
	~NativeReader() {}
	bool checkWine ();
	QString name();
	QString note();
	QString realName ();
	QString realNote ();
	bool setup(const QString &file);
	void setDvd(const QString &device, const QString &path);
	QString prefixPath ();
	QString wine ();
	QStringList components ();
	QString icon ();
	Prefix::ApplicationType type();
	Prefix *prefix();
	bool detectApp (QString path);
	bool needFile() const {return true;}
	bool needToSetMemory () const;
	Name nameForLang (QString locale);
	QStringList locales();
	QString defaultWine() const;
signals:
	void presetPrefixNeed(QString &prefix);
	void presetNameNeed (QString &name);
	void presetNoteNeed (QString &note);
protected:
	QSettings *s;
	void override_dll (const QString &dll, const QString &type);
	int possibleCount (QDir dir);
private:
	QString _name;
	QString _note;
	QString _prefix;
	QString _cdroot, _device;
	bool downloadWine();
	QString distr();
	void writeMD5(const QString &md5sum);
	QString getMD5();
	inline QString workdir();
	QStringList availableDiscs();
	QStringList discFileList (const QString &disc);
};

#endif
