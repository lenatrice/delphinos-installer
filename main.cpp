#include "mainWindow.hpp"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QFont font("DeJavu Sans");
    app.setFont(font);

    MainWindow window;
    window.show();


    return app.exec();
}