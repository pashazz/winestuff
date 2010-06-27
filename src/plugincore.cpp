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


#include "plugincore.h"

PluginWorker::PluginWorker(QObject *parent, corelib *core) :
    QObject(parent)
{
	foreach (QObject *plugin, QPluginLoader::staticInstances())
	{
		FormatInterface *interface = qobject_cast<FormatInterface*>(plugin);
		if (interface)
		{
			_plugins.append(interface);
			qDebug() << "pluginloader: Plugin " << interface->title() << " loaded.";
		}
	}
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
