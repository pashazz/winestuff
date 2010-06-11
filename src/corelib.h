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

#ifndef CORELIB_H
#define CORELIB_H

#include <QtCore>
#include <QtNetwork>
#include <QtSql>
#include "uiclient.h"
#include "libwinegame_global.h"
class  WINESTUFFSHARED_EXPORT corelib : public QObject
{
    Q_OBJECT
public:
	corelib(QObject *parent, UiClient *client);
	virtual ~corelib();
	static QString whichBin (QString bin);
	bool removeDir (const QString &dir);
	bool copyDir (const QString &dir, const QString &destination);
	void init (); /// этот метод на данный момент только прописывает видеопамять. В конфиг.
	bool unpackWine(QString distr, QString destination);
	QString unixSystem () {return system;} //наша замена QSysInfo. На системах Win/Lin/Mac/Symbian возвращает пустую строку
	bool syncPackages();
	static bool checkPrefixName (QString);
	void runSingleExe (QStringList exe) ;
	/// Блок настроек
	QString wineDir ();
	QString packageDir ();
	QString mountDir ();
	QString videoMemory ();
	QString discDir();
	bool forceFuseiso ();
	void setForceFuseiso(bool value, bool isempty = false);
	void syncSettings() {settings->sync();}
	static QString autorun (QString diskRoot);
	void setWineDir (QString dir, bool isempty =false);
	void setPackageDir (QString dir, bool isempty = false);
	void setMountDir (QString dir, bool isempty = false);
	void setVideoMemory (int memory, bool isempty = false);
	void setDiscDir(QString dir, bool isempty = false);
	void setSyncMirrors(QStringList urls, bool isempty = false);
	QStringList syncMirrors();
	QString getSudoProg ();
	QString downloadWine(QString url);
	QString shareDir () const;
	UiClient * client () {return ui;}
	int runGenericProcess(QProcess *process, const QString &program, QString message = "");
private slots:
	void error (QNetworkReply::NetworkError);
	void setRange (qint64, qint64); //заглушка для QProgressDialog
	void exitApp();
	void cancelCurrentOperation();

private:
	UiClient *ui;
	QSettings *settings;
	QSqlDatabase db;
	bool downloadExitCode;
	QString system;
	QNetworkReply *currentReply;
protected:
	bool initconf ();
	inline QString config();
	void initDb();
	void setConfigValue (QString key, QVariant value, bool setIfEmpty);
};

#endif // CORELIB_H
