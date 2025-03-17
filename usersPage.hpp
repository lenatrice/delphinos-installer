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
#include "statusIndicator.hpp"
#include <QFile>

class UsersPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* rootFormLayout;
    QFormLayout* userFormLayout;
    QTimer* passwordCheckTimer = nullptr;

    // Root user password
    StatusIndicator* rootLabelIndicator;
    QLabel* rootLabel;
    QLineEdit* rootPasswordLineEdit;
    QLineEdit* rootPasswordConfirmLineEdit;
    QHBoxLayout* rootPasswordsMatchLayout;
    StatusIndicator* rootPasswordsMatchIndicator;
    QLabel* rootPasswordsMatchLabel;


    // User configuration
    StatusIndicator* userLabelIndicator;
    QLabel* userLabel;
    QLineEdit* usernameLineEdit;
    QLineEdit* userPasswordLineEdit;
    QLineEdit* userPasswordConfirmLineEdit;
    QHBoxLayout* userPasswordsMatchLayout;
    StatusIndicator* userPasswordsMatchIndicator;
    QLabel* userPasswordsMatchLabel;

    QCheckBox* grantUserAdministrativePrivileges;

    StatusIndicator* usersConfiguredIndicator;
    QLabel* usersConfiguredLabel;

    QPushButton* confirmButton;

    bool isGroupNameUsed(const QString &username) {
        QFile file("/mnt/new_root/etc/group");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;
    
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith(username + ":")) {
                return true;
            }
        }
        return false;
    }

private slots:
    void onConfirmButtonClicked(bool checked);
    void onRootPasswordConfirmLineChanged(const QString& text);
    void onUserPasswordConfirmLineChanged(const QString& text);

public: 
    PageContent* getPage()
    {
        return page;
    };

    UsersPage(QWidget* parent);
};
