/*                      Delphinos Installer
              Copyright © Helena Beatrice Xavier Pedro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainWindow.hpp"
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QStringList>
#include <QStyleFactory>
#include <QStyleHints>
#include <QLoggingCategory>
#include <QFile>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char **argv)
{
    qDebug() << "Running delphinos-installer-elevated";
    qputenv("QT_QPA_PLATFORMTHEME", "qt6ct");
    QLoggingCategory::setFilterRules("qt.text.font.db=false");

    // Now the program is running as user 'delphinos-installer'
    QApplication app(argc, argv);

    qDebug() << "Available styles:" << QStyleFactory::keys();

    // Ensure we are running as user 'delphinos-installer'
    struct passwd *pw = getpwuid(geteuid());
    if (pw && QString(pw->pw_name) != "root") {
        QMessageBox::critical(nullptr, "Erro", "Esse programa precisa ser rodado com privilégios elevados.");
        return 1;
    }

    app.setStyle("qt6ct-style");

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

    QString styleSheet;
    
    QFile file(QApplication::applicationDirPath() + "/styleSheet.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        styleSheet = stream.readAll();
    } else {
        qWarning() << "Failed to open QSS file: " << QApplication::applicationDirPath() + "/styleSheet.qss";
    }
    
    MainWindow window;
    //window.setStyleSheet(styleSheet);
    window.show();

    return app.exec();
}
