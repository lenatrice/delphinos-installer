#include "mainWindow.hpp"
#include <QApplication>
#include <QDebug>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qDebug() << app.applicationVersion();

    QFont font("DeJavu Sans");
    app.setFont(font);

    MainWindow window;
    window.show();


    return app.exec();
}