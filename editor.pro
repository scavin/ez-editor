#-------------------------------------------------
#
# Project created by QtCreator 2011-03-15T23:25:05
#
#-------------------------------------------------

QT       += core gui

TARGET = editor

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        finddialog.cpp \
    fileassoc.cpp

HEADERS  += mainwindow.h \
        finddialog.h \
    fileassoc.h

TRANSLATIONS    +=  editor_zh_CN.ts

RESOURCES += res.qrc

RC_FILE = icon.rc
