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
    Q_PROPERTY (QString mountDir READ mountDir WRITE setMountDir );
    Q_PROPERTY (bool autosync READ autoSync WRITE setAutosync);
    Q_PROPERTY (bool forceFuseiso READ forceFuseiso WRITE setForceFuseiso);
    Q_PROPERTY (QString wineDir READ wineDir WRITE setWineDir);
    Q_PROPERTY (UiClient* uiClient READ client WRITE setClient);
    Q_PROPERTY (QString configPath READ configPath WRITE setConfigPath);
    Q_PROPERTY (QSqlDatabase database READ database WRITE setDatabase);

public:
	corelib(QObject *parent, UiClient *client, const QString &configPath);
	virtual ~corelib();
	void init (const QString &dbConnectionName = "");
	static QString whichBin (const  QString &bin);
	bool unpackWine(const QString &distr, const QString &destination);
	QString unixSystem () const {return system;}
	void  runSingleExe (const QStringList &exe);
	static bool checkPrefixName (const QString& prefix);
	/// Блок настроек
	QString wineDir () const;
	QString mountDir () const;
	QString videoMemory () const;
	bool autoSync () const;
	bool forceFuseiso () const;
	void setForceFuseiso(bool value, bool isempty = false);
	void syncSettings() const {settings->sync();}
	static QString autorun (const QString& diskRoot);
	void setWineDir (const QString &dir, bool isempty = false);
	void setMountDir (const QString &dir, bool isempty = false);
	void setVideoMemory (int memory, bool isempty = false);
	void setAutosync (bool value, bool isempty = false);
	QString configPath () const {return _confpath;}
	void setConfigPath (const QString &configPath) {_confpath = configPath;}
	QString getSudoProg () const;
	QString downloadWine(QString url, bool force = false);
	UiClient * client () const {return ui;}
	void setClient (UiClient *uiClient) {delete  ui; ui = uiClient;}
	int runGenericProcess(QProcess *process, const QString &program, QString message = "");
	void setDatabase (QSqlDatabase database) {
	    if (database.isOpen ())
		 db = database;}
	QSqlDatabase database () const {return db;}

signals:
	void videoMemoryChanged();
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
	QString _confpath;
	/* About copying files */
protected:
	bool initconf (const QString &configPath);
	inline QString config() const;
	void initDb();
	void setConfigValue (QString key, QVariant value, bool setIfEmpty);
};

#endif // CORELIB_H
