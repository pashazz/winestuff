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


#ifndef FORMATINTERFACE_H
#define FORMATINTERFACE_H
#include <QtCore>
#include "corelib.h"
#include "sourcereader.h"

namespace Pashazz
{
	enum Feautures
	{
		NoFeatures = 0, Detecting = 1, Multidisc = 2
	};
};
class FormatInterface
{
public:
	virtual ~FormatInterface() {}
	virtual QString title () = 0;
	virtual QString author () = 0;
	virtual bool hasFeature (Pashazz::Feautures feature) = 0;
	virtual QList<SourceReader *> readers (corelib *core, bool includeDvd = false) = 0;
	virtual bool updateAllWines (corelib *core) = 0;
	virtual SourceReader * readerById (const QString &id, corelib *core) = 0;
};
Q_DECLARE_INTERFACE(FormatInterface,
					"org.pashazz.winestuff.FormatInterface/0.2")
#endif // FORMATINTERFACE_H
