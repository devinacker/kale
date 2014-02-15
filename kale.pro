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

# OS-specific metadata and stuff
win32:RC_FILE = src/windows.rc

SOURCES += \
    src/romfile.cpp \
    src/mapscene.cpp \
    src/mapchange.cpp \
    src/mainwindow.cpp \
    src/level.cpp \
    src/compress.c \
    src/main.cpp \
    src/tileset.cpp \
    src/coursewindow.cpp \
    src/stuff.cpp \
    src/sceneitem.cpp \
    src/graphics.cpp \
    src/propertieswindow.cpp \
    src/hexspinbox.cpp \
    src/tilesetview.cpp \
    src/tileeditwindow.cpp \
    src/spriteeditwindow.cpp

HEADERS  += \
    src/romfile.h \
    src/mapscene.h \
    src/mapchange.h \
    src/mainwindow.h \
    src/level.h \
    src/compress.h \
    src/version.h \
    src/graphics.h \
    src/tileset.h \
    src/coursewindow.h \
    src/stuff.h \
    src/sceneitem.h \
    src/propertieswindow.h \
    src/hexspinbox.h \
    src/tilesetview.h \
    src/tileeditwindow.h \
    src/spriteeditwindow.h

FORMS += \
    src/mainwindow.ui \
    src/coursewindow.ui \
    src/propertieswindow.ui \
    src/tileeditwindow.ui \
    src/spriteeditwindow.ui

RESOURCES += \
    src/icons.qrc

OTHER_FILES += \
    src/windows.rc \
    TODO.txt
