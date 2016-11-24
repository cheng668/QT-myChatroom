#-------------------------------------------------
#
# Project created by QtCreator 2016-11-24T13:06:47
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myChatroom
TEMPLATE = app


SOURCES += main.cpp \
    drawer.cpp \
    chatroom.cpp

HEADERS  +=  drawer.h \
    chatroom.h

FORMS    +=  drawer.ui \
    chatroom.ui

RESOURCES += \
    images.qrc
