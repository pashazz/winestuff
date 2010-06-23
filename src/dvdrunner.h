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

#ifndef DVDRUNNER_H
#define DVDRUNNER_H

#include <QtCore>
#include "corelib.h"
#include "inireader.h"
namespace Pashazz
{
enum DriveType
{
	Real = 1, Image = 2, Unknown = 0
};
};

class WINESTUFFSHARED_EXPORT  DVDRunner : public QObject
{
	Q_OBJECT
public:
	DVDRunner(corelib *lib, QString path);
	virtual ~DVDRunner() {}
	QString diskDirectory(); //возвращает пустую строку, если монтирование завершилось неудачно
	QString exe();
	Pashazz::DriveType objectType () {return type;}
	SourceReader *sourceReader () {return this->reader;}
	bool success() {return result;} //Закончилось ли распознавание успешно
	void setReader (SourceReader *reader);
	void cleanup ();
	bool isMounted () {return mounted;}
public slots:
	void cancel();
signals:
	void insertNextCd (bool &result, int cd); //Пользователь должен вставить CD.
	void fileNeed (QString &exe, QString disc);
private:
	corelib *core;
	bool result;
	bool multidisc;
	bool cancelled;
	bool detect();
	SourceReader * detectBy (QString diskPath); //главная функция
	bool checkDisc(QString &diskPath);
	bool prepare (bool nodetect = false); //метод для выполнения различных подготовок (монтирования и т.д.). Если WineGame распознал диск сам, то этот метод вызывается из конструктора.
	SourceReader *reader;
	QString diskPath, realDrive;
	QString mount, umount;
	Pashazz::DriveType type;
	int _max; //copyfile size
	bool mounted;
	QStringList entrylist;
private slots:
};

#endif // DVDRUNNER_H

