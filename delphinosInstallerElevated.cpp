#include "mainWindow.hpp"
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QStringList>
#include <QStyleFactory>
#include <QStyleHints>
#include <QFile>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char **argv)
{
    qDebug() << "Running delphinos-installer-elevated";

    QProcess echoEnv;
    QStringList echoArgs;
    echoEnv.start("echo", echoArgs);

    // Now the program is running as user 'delphinos-installer'
    QApplication app(argc, argv);

    qDebug() << "Available styles:" << QStyleFactory::keys();

    // Ensure we are running as user 'delphinos-installer'
    struct passwd *pw = getpwuid(geteuid());
    if (pw && QString(pw->pw_name) != "root") {
        QMessageBox::critical(nullptr, "Erro", "Esse programa precisa ser rodado com privilégios elevados.");
        return 1;
    }

    app.setStyle("Breeze");

    // Create a dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::WindowText, QColor(199, 206, 217));
    darkPalette.setColor(QPalette::Base, QColor(39, 41, 47));
    darkPalette.setColor(QPalette::AlternateBase, QColor(49, 49, 63));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(39, 41, 47));
    darkPalette.setColor(QPalette::ToolTipText, QColor(199, 206, 217));
    darkPalette.setColor(QPalette::Text, QColor(199, 206, 217));
    darkPalette.setColor(QPalette::Button, QColor(39, 41, 47));
    darkPalette.setColor(QPalette::ButtonText, QColor(199, 206, 217));
    darkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(27, 29, 33));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(57, 59, 64));
    darkPalette.setColor(QPalette::BrightText, QColor(230, 234, 240));
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 86, 199));
    darkPalette.setColor(QPalette::HighlightedText, QColor(230, 234, 240));

    app.setPalette(darkPalette);

    MainWindow window;

    window.setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid rgb(72, 80, 90);" // Dark border color for text edit
        "  background-color: #272933;"  // Text area background
        "  color: rgb(199, 206, 217);"             // Text color inside text area
        "  border-radius: 2px;"            // Rounded corners for button
        "  height: 24px;"                        // Adjust the height of the text box"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid rgb(55, 142, 255);" 
        "  background-color:rgb(67, 70, 85);"            // Background when focused
        "  color: rgb(199, 206, 217);"           // Text color when focused
        "  border-radius: 2px;"                   // Rounded corners for button
        "}"
        "QLineEdit:selected {"
        "  border: 1px solid rgb(55, 142, 255);" // Dark border color for text edit
        "  background-color: #272933;"  // Text area background
        "  color: rgb(199, 206, 217);"             // Text color inside text area
        "  border-radius: 2px;"            // Rounded corners for button
        "}"
        "QListWidget {"
        "  border: 1px solid rgb(72, 80, 90);"
        "  border-radius: 2px;"            // Rounded corners for button
        "}"
        "QListWidget::item:selected {"
        "  background-color: rgb(55, 142, 255);"   // Background color when an item is selected
        "  color: #fff;"                          // Text color when an item is selected
        "}"
        "QTableWidget {"
        "  border: 1px solid rgb(72, 80, 90);"
        "  border-radius: 2px;"            // Rounded corners for button
        "}"
        "QTableWidget::item:selected {"
        "  background-color: 1px solid rgb(55, 142, 255);"  
        "  padding: 0px;"                        // Padding for list items
        "}"
    );
    window.show();

    return app.exec();
}