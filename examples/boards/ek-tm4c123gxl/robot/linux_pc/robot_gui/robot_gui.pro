#-------------------------------------------------
#
# Project created by QtCreator 2016-04-27T20:53:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = robot_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logswindow.cpp \
    logsdialog.cpp \
    eventhandler.cpp \
    settingsdialog.cpp \
    settings.cpp

HEADERS  += mainwindow.h \
    logswindow.h \
    logsdialog.h \
    eventhandler.h \
    settingsdialog.h \
    settings.h

INCLUDEPATH += "../"

FORMS    += mainwindow.ui \
    logswindow.ui \
    logsdialog.ui \
    settingsdialog.ui

LIBS += -L../lib -lrobot

RESOURCES     = resource.qrc


CONFIG += c++11
