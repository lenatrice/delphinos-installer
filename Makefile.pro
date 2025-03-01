QT += core gui
QT += widgets
QT += dbus
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets
CONFIG += c++17

CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0 gio-2.0

TARGET = "delphinos-installer"
TEMPLATE = app

SOURCES += main.cpp \
    mainWindow.cpp

HEADERS  += mainWindow.hpp

INCLUDEPATH += /usr/include/glib-2.0 /usr/lib/glib-2.0/include/ /usr/include/libnm/
