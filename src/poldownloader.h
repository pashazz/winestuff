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
/*!
  PolDownloader is a http://wine.playonlinux.com/linux-i386/ download & integration client
  */
#ifndef POLDOWNLOADER_H
#define POLDOWNLOADER_H

#include <QtCore>
#include <QtNetwork>
#include "prefixcollection.h"

class PolDownloader : public QObject
{
	Q_OBJECT
public:
	PolDownloader(PrefixCollection *collection, const QString &prefixId, corelib *lib);
	QStringList versionList() {return versions;}
	bool setWineVersion (QString version); //return false if SQL error or no such version
	QString detectCurrentVersion();

public slots:
	void fallback();

private:
	PrefixCollection *pcoll;
	corelib *core;
	Prefix *prefix;
	bool goodGet;
	bool downloadWine(QString version);
	bool checkSHA1(QString file);
	QNetworkReply *currentReply;
	QStringList sha1sums;
	QStringList versions;
	QStringList files;

private slots:
	void cancelCurrentOperation();
protected:
	 QString URL;

private slots:
	 void	setProgressRange (qint64, qint64);
	 void	error (QNetworkReply::NetworkError);
};

#endif // POLDOWNLOADER_H
