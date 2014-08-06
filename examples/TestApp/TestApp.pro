#-------------------------------------------------
#
# Project created by QtCreator 2014-04-27T19:27:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestApp
TEMPLATE = app

# uncomment the following line for static linking
CONFIG += qsrmail_static
include(../../qsrmail.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    filelistmodel.cpp

HEADERS  += mainwindow.h \
    filelistmodel.h

FORMS    += mainwindow.ui

