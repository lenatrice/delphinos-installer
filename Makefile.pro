QT += core gui
QT    += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets
CONFIG += c++17

TARGET = "delphinos-installer"
TEMPLATE = app

SOURCES += main.cpp \
    mainWindow.cpp

HEADERS  += mainWindow.hpp