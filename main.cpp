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

#include <QProcess>
#include <QMessageBox>
#include <QStringList>
#include <QApplication>
#include <QObject>
#include <unistd.h>
#include <pwd.h>

bool isRunningAsRoot() {
    struct passwd *pw = getpwuid(geteuid());
    return (pw && QString(pw->pw_name) == "root");
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QProcess process;  // Use a regular QProcess instead of std::unique_ptr

    qDebug() << "Running delphinos-installer";

    // If the program is not running as delphinos-installer, use pkexec to elevate privileges
    if (!isRunningAsRoot()) {
        qDebug() << "Not running as root";
        QStringList pkexecArguments;
        pkexecArguments << "sh" << "-c"
                           << "env DISPLAY=" + qgetenv("DISPLAY") +
                           " XAUTHORITY=" + qgetenv("XAUTHORITY") + 
                           " " + QCoreApplication::applicationDirPath() + "/delphinos-installer-elevated";

        QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process](){
            QByteArray output = process.readAllStandardOutput();
            QString decodedOutput = QString::fromUtf8(output);
            qDebug() << decodedOutput;
        });
        
        QObject::connect(&process, &QProcess::readyReadStandardError, [&process](){
            QByteArray output = process.readAllStandardError();
            QString decodedOutput = QString::fromUtf8(output);
            qDebug() << decodedOutput;
        });
        
        QObject::connect(&process, &QProcess::finished, [&process](int exitCode, QProcess::ExitStatus status){
            qDebug() << "Process finished with exit code" << exitCode;
            if (status == QProcess::CrashExit) {
                qDebug() << "The process crashed!";
            }
        });

        qDebug() << "Starting pkexec";
                           
        // Start the process using pkexec to launch the second executable
        process.start("pkexec", pkexecArguments);

        // Check if the process started successfully
        if (!process.waitForStarted()) {
            // If the process didn't start successfully, show an error
            QMessageBox::critical(nullptr, "Error", "Failed to start pkexec.");
            return 1;
        }

        // Wait for the process to finish with no timeout
        if (!process.waitForFinished(-1)) {
            // If the process didn't finish successfully, show an error
            QMessageBox::critical(nullptr, "Error", "Failed to authenticate using Polkit.");
            return 1;
        }

    } else {
        process.start(QCoreApplication::applicationDirPath() + "/delphinos-installer-elevated");
        
        if (!process.waitForStarted()) {
            // If the process didn't start successfully, show an error
            QMessageBox::critical(nullptr, "Error", "Failed to start the installer.");
            return 1;
        }

        // Wait for the process to finish with no timeout
        if (!process.waitForFinished(-1)) {
            // If the process didn't finish successfully, show an error
            QMessageBox::critical(nullptr, "Error", "Failed to authenticate using Polkit.");
            return 1;
        }
    }

    // This ensures the main loop doesn't exit prematurely
    return 0;
}
