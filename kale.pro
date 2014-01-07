#-------------------------------------------------
#
# Project created by QtCreator 2014-01-04T21:49:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CFLAGS += -std=c99

# Use the latter, if your compiler supports it
#QMAKE_CXXFLAGS += -std=c++0x -U__STRICT_ANSI__
QMAKE_CXXFLAGS += -std=c++11

TARGET = kale
TEMPLATE = app


SOURCES += \
    src/romfile.cpp \
    src/mapscene.cpp \
    src/mapchange.cpp \
    src/mainwindow.cpp \
    src/level.cpp \
    src/compress.c \
    src/main.cpp

HEADERS  += \
    src/romfile.h \
    src/mapscene.h \
    src/mapchange.h \
    src/mainwindow.h \
    src/level.h \
    src/compress.h \
    src/version.h \
    src/graphics.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    src/icons.qrc

OTHER_FILES += \
    src/windows.rc
