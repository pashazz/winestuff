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

#ifndef NATIVE_H
#define NATIVE_H
#include "formatinterface.h"
#include "inireader.h"
class NativeFormat : public QObject, public FormatInterface
{
	Q_OBJECT
	Q_INTERFACES(FormatInterface)
public:
    NativeFormat();
	QList <SourceReader *> readers(corelib *core, bool includeDvd);
	QString author() {return "(C) Pavel Zinin - 2010. GNU LGPL licensed.";}
	QString title () {return "Winestuff native format";}
	bool hasFeature(Pashazz::Feautures feature);
	bool updateAllWines(corelib *core);
	SourceReader * readerById (const QString &id, corelib *core);
private:
	//Todo: packageDirs here
};

#endif // NATIVE_H
