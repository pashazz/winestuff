# -------------------------------------------------
# Project created by QtCreator 2010-04-18T14:01:01
# -------------------------------------------------
QT += network \
    sql
QT -= gui
TARGET = winestuff
TEMPLATE = lib
DEFINES += WINESTUFF_LIBRARY
SOURCES += corelib.cpp \
    prefix.cpp \
    dvdrunner.cpp \
    poldownloader.cpp
HEADERS += libwinegame_global.h \
    corelib.h \
    prefix.h \
    uiclient.h \
    dvdrunner.h \
    poldownloader.h
isEmpty ($$PREFIX)
:PREFIX = /usr
target.path = $$PREFIX/lib
INSTALLS += target
VERSION = 0.1.0
TRANSLATIONS += ../l10n/lwg_ru.ts
