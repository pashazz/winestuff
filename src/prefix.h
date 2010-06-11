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
/// класс для работы с префиксами (не preset)
#include <QtCore>
#include "libwinegame_global.h"
#include "corelib.h"
 class WINESTUFFSHARED_EXPORT  Prefix : public QObject
{
Q_OBJECT

public:
	explicit Prefix(QObject *parent, QString workdir, corelib *lib);
	QString prefixPath () {return _path;}
    QString prefixName () {return _prefix;}
    QString wine();
    QString standardExe();
   QString name ();
   corelib *myLib() {return core;}
   QString note ();
  QString setup (); //application/setup value
   QString distr(); //wine/distr value
   bool runApplication (QString exe, QString diskroot = "", QString imageFile = ""); //well, it`s helper.
  void makeDesktopIcon (const QString &path, const  QString &name);
  bool isPreset();
  bool isMulti();
   short int discCount();
  QString projectWorkingDir () {return _workdir;}
  QProcessEnvironment envs () {return env;}
  //Запуск программы в данном Prefix
  void runProgram (QString exe);
  void lauchWinetricks(QStringList args);
  void setMemory();
  void removePrefix ();
  bool installFirstApplication ();
  bool checkWineDistr();
  bool hasDBEntry ();
  QString getExeWorkingDirectory (QString);
public slots:
  void lauch_c();


signals:
void prefixNameNeed (QString &name);
private:
bool downloadCancelled;
QString _prefix;
QString _path;
QString _workdir;
  QSettings *s;
QProcessEnvironment env;
bool downloadWine ();
corelib *core;
void getPrefixPath();
QSqlDatabase db;
QString getMD5();
void writeMD5(QString md5sum);
protected:
void makefix (); //исправление реестра Wine: запуск winebrowser.exe -nohome %1 вместо winebrowser.exe -nohome
void makeWineCdrom (const  QString &path, const QString &device= "/dev/cdrom");

};
#endif // PREFIX_H
