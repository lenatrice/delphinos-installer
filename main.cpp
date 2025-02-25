#include "QtGui.hpp"
#include <QApplication>

int main(int argc, char** argv)
{
    std::cout << "Teste" << std::endl;
    QApplication app(argc, argv);
    MainWindow window;
    window.show();


    return app.exec();
}