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

#include "prefixcollection.h"

PrefixCollection::PrefixCollection (QSqlDatabase database, corelib *lib, PluginWorker *worker, QObject *parent)
	:QObject(parent)
{
	db = database;
	wrk = worker;
	core =lib;
	loc = QLocale::system().name();
	connect (lib, SIGNAL(videoMemoryChanged()), this, SLOT(updateVideoMemory()));
}

Prefix* PrefixCollection::install(SourceReader *reader, QString file, QString dvdObj)
{
	if (!reader)
		return 0;

	if (reader->prefixPath().isEmpty())
		return 0;
	if (reader->name().isEmpty())
		return 0;

	//проверяем Wine
	if(!reader->checkWine())
		return 0;
	if (havePrefix(reader->ID()))
	{
		Prefix *prefix (getPrefix(reader->ID()));
		prefix->runApplication(file, "", true);
		return prefix;
	}
	/* Записываем Prefix в базу данныхъ */
	QSqlQuery q (db);
	q.prepare("INSERT INTO Apps (prefix, wineprefix, wine) VALUES (:prefix, :wineprefix, :wine)");
	q.bindValue(":prefix", reader->ID());
	q.bindValue(":wineprefix", reader->prefixPath());
	q.bindValue(":wine", reader->wine());
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return 0;
	}
	foreach (QString locale, reader->locales())
	{
	Name name = reader->nameForLang(locale);
	q.prepare("INSERT INTO Names (prefix, name, note, lang) VALUES (:prefix, :name, :note, :lang)");
	q.bindValue(":prefix", reader->ID());
	q.bindValue(":name", name.first);
	q.bindValue(":note", name.second);
	q.bindValue(":lang", locale);
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return 0;
	}
}
	Prefix *pref = reader->prefix();
	if (reader->needToSetMemory())
		pref->setMemory();
	if (!dvdObj.isEmpty ())
	{
		/* cdrom working */
		QString cdroot, image;
		QFileInfo dvdfi (dvdObj);
		if (dvdfi.isFile ())
		{
			image = dvdObj;
			cdroot = core->mountDir ();
		}
		else if (dvdfi.isDir ())
		{
			image = "/dev/cdrom";
			cdroot = dvdObj;
		}
		if (!cdroot.isEmpty ())
			reader->setDvd(image, cdroot);
    }
	pref->makefix();
	//launch winetricks
	launchWinetricks(pref, reader->components());
	reader->setup(file);
	return pref;
}

void PrefixCollection::launchWinetricks(Prefix *prefix, const QStringList &args)
{
	QProcess *p = new QProcess (this);
	p->setProcessEnvironment(prefix->environment());
	qDebug() << "List of components that must be installed: " << args.join(" ");
	foreach (QString arg, args)
	{
		core->runGenericProcess(p, "winetricks -q " + arg, tr("Installing component: %1").arg(arg));
	}
}

QList <Prefix*> PrefixCollection::prefixes()
{
	QList <Prefix*> list;
	QSqlQuery q(db);
	q.prepare("SELECT prefix FROM Apps");
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return list;
	}
	while (q.next())
	{
		list.append(getPrefix(q.value(0).toString()));
	}
	return list;
}

Name PrefixCollection::getName(QString locale, QString ID)
{
	Name names;
	// First of all, check if this locale exists
	QSqlQuery q(db);
	q.prepare("SELECT COUNT (id) FROM Names WHERE lang=? AND prefix=?");
	q.addBindValue(locale);
	q.addBindValue(ID);
	if ((!q.exec()) || (!q.first()))
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return names;
	}
	if ((q.value(0).toInt() == 0) && (locale != "C"))
	{
		return getName("C", ID);
	}
	//получаем name и note
	q.prepare("SELECT name, note FROM Names WHERE lang=:lang AND prefix=:prid");
	q.bindValue(":lang", locale);
	q.bindValue(":prid", ID);
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
	}
	q.first();
	names.first = q.value(0).toString();
	names.second = q.value(1).toString();
	return names;
}

Prefix* PrefixCollection::getPrefix(QString id)
{
	if (id.isEmpty())
		return 0;
	Prefix *prefix = new Prefix (this, core);
	QSqlQuery q(db);
	q.prepare("SELECT wineprefix, wine FROM Apps WHERE prefix=:id");
	q.bindValue(":id", id);
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return 0;
	}
	if (!q.first())
		return 0;
	prefix->setID(id);
	prefix->setPath(q.value(0).toString());
	prefix->setWine(q.value(1).toString());
	Name names = getName(loc.name(), id);
	prefix->setName(names.first);
	prefix->setNote(names.second);
	return prefix;
}

bool PrefixCollection::updatePrefix(Prefix *newData, QString id)
{
	/// WARNING: If you use this function, then name and note of this prefix is WILL UNCHANGED!
	if (newData->ID().isEmpty())
		return false;
	if (id.isEmpty())
		id = newData->ID();
	QSqlQuery q(db);
	q.prepare("UPDATE Apps SET prefix=:prefix, wineprefix=:wineprefix, wine=:wine WHERE prefix=:id");
	q.bindValue(":prefix", newData->ID());
	q.bindValue(":wineprefix", newData->path());
	q.bindValue(":wine", newData->wine());
	q.bindValue(":id", id);
	if (!q.exec())
	{
		core->client()->error(tr("Database error"), tr("Traceback: %1, query: %2").arg(q.lastError().text(), q.lastQuery()));
		return 0;
	}
	return true;
}

bool PrefixCollection::remove(QString id)
{
	Prefix *prefix = getPrefix(id);
	if (prefix->path().isEmpty() || prefix->ID().isEmpty())
		return false;
	QProcess p;
	core->runGenericProcess(&p, QString("rm -rf %1").arg(prefix->path()), tr("Removing prefix %1").arg(prefix->name()));
	QSqlQuery q(db);
	q.prepare("DELETE FROM Apps WHERE prefix=:pr");
	q.bindValue(":pr", id);
   if (!q.exec())
	{
	   qDebug() << "WARNING: Unable to execute query for delete Prefix";
	   return false;
   }
   return true;
}

bool PrefixCollection::havePrefix(const QString &id)
{
	if (id.isEmpty())
		return false;
	Prefix *prefix = getPrefix(id);
	if (!prefix)
		return false;
	return (!prefix->ID().isEmpty()) || (!prefix->path().isEmpty());
}

void PrefixCollection::updateVideoMemory()
{
	foreach (Prefix *prefix, prefixes())
	{
		QString id = prefix->ID();
		SourceReader *reader;
		foreach (FormatInterface *plugin, wrk->plugins())
		{
			SourceReader *r = plugin->readerById(id, core);
			if (r)
			{
				reader = r;
				break;
			}
		}
		if (reader)
		{
			if (reader->needToSetMemory())
				prefix->setMemory();
		}
		else
			prefix->setMemory();
	}
}
