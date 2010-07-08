/*
 wst-listgen - utilty for creating lists for winestuff`s native plug-in
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
  usage: wst-listgen -d [dir] > [output_file]
	  or: wst-listgen -d [dir] -o output file

  */
#include <QtCore>

QStringList dirList(QDir dir)
{
	static QDir mainPath = dir; //присваивается только 1 раз.
	QStringList myList = QStringList();
	foreach (QFileInfo directory, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
	{
		myList.append(mainPath.relativeFilePath(directory.absoluteFilePath()));
		if (directory.isDir())
			myList.append(dirList(QDir(directory.absoluteFilePath())));
	}
	return myList;
}

void showUsageMessage ()
{
	QTextStream out (stdout);
	out << QString("Usage: %1 -d [target directory] -o [output file]").arg(qApp->arguments().at(0)) << endl;
	out << "Special options:" << endl;
	out << "--help - show this message" << endl;
}

QString getTargetDir (const QStringList &args)
{
	int index = args.indexOf("-d");
	if (index == -1)
		return "";
	if (index >= args.count() - 1) //это последний аргумент.
		return "";
	//get absolute file path
	QFileInfo tdir (args.at(index+1));
	if (tdir.exists())
		return tdir.absoluteFilePath();
	else
		return "";
}

QString getOutputFile (const QStringList &args)
{
	int index = args.indexOf("-o");
	if (index == -1)
		return "";
	if (index >= args.count() - 1) //это последний аргумент.
		return "";
	return args.at(index + 1);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
	//init encodings
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForTr(codec);

	// analyze commandline arguments
	QStringList args  = a.arguments();
	args.removeFirst();

	if (args.count() == 0 || args.at(0) == "--help")
	{
		showUsageMessage();
		a.quit();
	}
	//initialize stream
	QString streamTarget = getOutputFile(args);
	QFile file;
	QTextStream out (&file);
	if (streamTarget.isEmpty() || streamTarget == "-")
	{
		//writing to stdout
		file.open(stdout, QIODevice::WriteOnly);
	}
	else
	{
		//writing to file.
		file.setFileName(streamTarget);

		if (!file.open(QIODevice::WriteOnly))
		{
			//refer
			qDebug() << a.arguments().at(0) << ": invalid file argument. Type " <<  a.arguments().at(0) << " --help to help";
			return -1;
		}
	}

	//get directory
	QString dir = getTargetDir(args);
	if (dir.isEmpty())
	{
		qDebug() << a.arguments().at(0) << ": invalid directory argument. Type " <<  a.arguments().at(0) << " --help to help";
		return -2;
	}

	QStringList list  = dirList(QDir(dir));
	out << list.join("\n");

	file.close();
	return 0; //we dont execute qt loop
}
