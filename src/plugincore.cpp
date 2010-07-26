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

#include "plugincore.h"

PluginWorker::PluginWorker(QObject *parent, corelib *core) :
    QObject(parent)
{
	//! we will NOT load static plugins
	QDir dir (qApp->applicationDirPath()); //calculate pkgdir
	if (dir.dirName() == "bin")
	{
		dir.cdUp();
		dir.cd("lib/winestuff/plugins");
	}
	else if (dir.dirName() == "lib")
		dir.cd("winestuff/plugins");
	else
		dir.setPath(core->configPath() + "/plugins");

	if (dir.exists())
		plugdir = dir.absolutePath();

	//load dynamic plugins
	foreach (QFileInfo file, dir.entryInfoList(QDir::Files))
	{
		QPluginLoader loader (file.filePath());
		QObject *plugin = loader.instance();
		if (!plugin)
			continue;
		FormatInterface *interface = qobject_cast<FormatInterface*>(plugin);
		if (interface)
		{
			_plugins.append(interface);
			interface->on_loadPlugin(core);
			fileNames << file.fileName();
			qDebug() << "pluginloader: Plugin" << interface->title() << " loaded.";
		}
		else
			qDebug() << "pluginloader: Can not load plugin from" << file.fileName();
	}
}


QStringList PluginWorker::files()
{
	return fileNames;
}

QList<FormatInterface*> PluginWorker::plugins()
{
	return _plugins;
}

SourceReader* PluginWorker::reader(const QString &id)
{
	foreach (FormatInterface *plugin, _plugins)
	{
		SourceReader *reader = plugin->readerById(id);
		if (reader)
			return reader;
	}
	return 0;
}
