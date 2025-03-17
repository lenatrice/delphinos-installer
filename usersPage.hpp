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
