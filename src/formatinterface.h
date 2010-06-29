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
	virtual void on_loadPlugin(corelib *lib) = 0;
	virtual QString title () = 0;
	virtual QString author () = 0;
	virtual bool hasFeature (Pashazz::Feautures feature) = 0;
	virtual QList<SourceReader *> readers (bool includeDvd = false) = 0;
	virtual bool updateAllWines () = 0;
	virtual SourceReader * readerById (const QString &id) = 0;
};
Q_DECLARE_INTERFACE(FormatInterface,
					"org.pashazz.winestuff.FormatInterface/0.2")
#endif // FORMATINTERFACE_H
