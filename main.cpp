#include "mainWindow.hpp"
#include <QApplication>
#include <QDebug>
#include <QLoggingCategory>

int main(int argc, char** argv)
{

    QLoggingCategory::setFilterRules("qt.qpa.fonts=true");
    QApplication app(argc, argv);

    MainWindow window;
    window.show();


    return app.exec();
}