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

#include "usersPage.hpp"
#include <QLineEdit>
#include <QCheckBox>
#include <QPointer>
#include <QProcess>
#include <QRegularExpression>

UsersPage::UsersPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent("Criação de usuário", "Defina a senha para o usuário root e crie o seu usuário para poder logar no sistema.", 370, 580, this);

    rootFormLayout = new QFormLayout;
    rootFormLayout->setAlignment(Qt::AlignHCenter);
    userFormLayout = new QFormLayout;
    userFormLayout->setAlignment(Qt::AlignHCenter);

    page->setCanAdvance(false);
    
    // Root user password
    QHBoxLayout* rootLabelLayout = new QHBoxLayout;
    rootLabel = new QLabel("<b>Usuário root</b>\n(possui controle irrestrito sobre o sistema)");
    rootLabel->setWordWrap(true);
    rootLabelLayout->addWidget(rootLabel);
    rootFormLayout->addRow(rootLabelLayout);
    
    rootPasswordLineEdit = new QLineEdit;
    rootPasswordLineEdit->setEchoMode(QLineEdit::Password);
    rootPasswordConfirmLineEdit = new QLineEdit;
    rootPasswordConfirmLineEdit->setEchoMode(QLineEdit::Password);
    
    rootPasswordsMatchLayout = new QHBoxLayout;
    QWidget* rootPasswordsMatchWidget = new QWidget;
    rootPasswordsMatchWidget->setLayout(rootPasswordsMatchLayout);
    rootPasswordsMatchIndicator = new StatusIndicator;
    rootPasswordsMatchLabel = new QLabel;
    rootPasswordsMatchLabel->setAlignment(Qt::AlignRight);
    rootPasswordsMatchLayout->addWidget(rootPasswordsMatchIndicator);
    rootPasswordsMatchLayout->addWidget(rootPasswordsMatchLabel);
    
    connect(rootPasswordConfirmLineEdit, &QLineEdit::textChanged, this, &UsersPage::onRootPasswordConfirmLineChanged);
    rootFormLayout->addRow("Senha:", rootPasswordLineEdit);
    rootFormLayout->addRow("Confirmar senha:", rootPasswordConfirmLineEdit);
    rootFormLayout->addRow(rootPasswordsMatchLayout);

    page->addLayout(rootFormLayout);
    page->addWidget(rootPasswordsMatchWidget, 0, Qt::AlignRight);
    page->addStretch();
    
    // User configuration
    QHBoxLayout* userLabelLayout = new QHBoxLayout;
    userLabel = new QLabel("<b>Crie o seu usuário</b>");
    userLabelLayout->addWidget(userLabel);
    userFormLayout->addRow(userLabelLayout);

    usernameLineEdit = new QLineEdit;
    userPasswordLineEdit = new QLineEdit;
    userPasswordLineEdit->setEchoMode(QLineEdit::Password);
    userPasswordConfirmLineEdit = new QLineEdit;
    userPasswordConfirmLineEdit->setEchoMode(QLineEdit::Password);
    connect(userPasswordConfirmLineEdit, &QLineEdit::textChanged, this, &UsersPage::onUserPasswordConfirmLineChanged);

    userPasswordsMatchLayout = new QHBoxLayout;
    QWidget* userPasswordsMatchWidget = new QWidget;
    userPasswordsMatchWidget->setLayout(userPasswordsMatchLayout);
    userPasswordsMatchLayout->setAlignment(Qt::AlignRight);
    userPasswordsMatchIndicator = new StatusIndicator;
    userPasswordsMatchLabel = new QLabel;
    userPasswordsMatchLabel->setAlignment(Qt::AlignRight);
    userPasswordsMatchLayout->addWidget(userPasswordsMatchIndicator);
    userPasswordsMatchLayout->addWidget(userPasswordsMatchLabel);
    
    // Whether to grant administrative privileges to the user
    grantUserAdministrativePrivileges = new QCheckBox;
    grantUserAdministrativePrivileges->setText("Conceder privilégios administrativos ao usuário.");
    grantUserAdministrativePrivileges->setChecked(true);
    
    userFormLayout->addRow("Nome de usuário:", usernameLineEdit);
    userFormLayout->addRow("Senha:", userPasswordLineEdit);
    userFormLayout->addRow("Confirmar senha:", userPasswordConfirmLineEdit);

    page->addLayout(userFormLayout);
    page->addWidget(userPasswordsMatchWidget, 0, Qt::AlignRight);
    page->addWidget(grantUserAdministrativePrivileges);
    page->addStretch();


    
    // Confirmation button
    confirmButton = new QPushButton("Confirmar");
    connect(confirmButton, &QPushButton::clicked, this, &UsersPage::onConfirmButtonClicked);
    page->addWidget(confirmButton, 0, Qt::AlignRight);
}

void UsersPage::onRootPasswordConfirmLineChanged(const QString& text)
{
    QTimer* timer = new QTimer;
    timer->start(3500);
    connect(timer, &QTimer::timeout, this, [this](){
        if (rootPasswordConfirmLineEdit->text().isEmpty())
        {
            rootPasswordsMatchIndicator->setStatus(StatusIndicator::None);
            rootPasswordsMatchLabel->hide();
        }
        if (rootPasswordLineEdit->text() != rootPasswordConfirmLineEdit->text())
        {
            rootPasswordsMatchIndicator->setStatus(StatusIndicator::Error);
            rootPasswordsMatchLabel->setText("<span style='color: rgb(212, 62, 62);'>As senhas não conferem</span>");
            rootPasswordsMatchLabel->show();
        } else {
            rootPasswordsMatchLabel->hide();
            rootPasswordsMatchIndicator->setStatus(StatusIndicator::Ok);
        }
    });
}

void UsersPage::onUserPasswordConfirmLineChanged(const QString& text)
{
    QTimer* timer = new QTimer;
    timer->start(3500);
    connect(timer, &QTimer::timeout, this, [this](){
        if (userPasswordConfirmLineEdit->text().isEmpty())
        {
            userPasswordsMatchIndicator->setStatus(StatusIndicator::None);
            userPasswordsMatchLabel->hide();
        }

        if (userPasswordLineEdit->text() != userPasswordConfirmLineEdit->text())
        {
            userPasswordsMatchIndicator->setStatus(StatusIndicator::Error);
            userPasswordsMatchLabel->show();
            userPasswordsMatchLabel->setText("<span style='color: rgb(212, 62, 62);'>As senhas não conferem</span>");
        } else {
            userPasswordsMatchLabel->hide();
            userPasswordsMatchIndicator->setStatus(StatusIndicator::Ok);
        } 
    });
}

void UsersPage::onConfirmButtonClicked(bool checked)
{
    const QString& username = usernameLineEdit->text();
    const QString& rootPassword = rootPasswordLineEdit->text();
    const QString& userPassword = userPasswordLineEdit->text();

    const QList<QString> invalidUsernames = QList<QString>
    {
        "root", "bin", "daemon", "sys", "sync", "shutdown", "halt", "nobody",
        "dbus", "http", "ftp", "mail", "postfix", "sshd", "systemd-network",
        "systemd-resolve", "polkitd", "avahi", "rpc", "cups", "ntp", "tor",
        "gdm", "colord", "uuidd", "sddm", "rfkill", "systemd-coredump",
        "systemd-bus-proxy", "tss", "usbmux", "git", "dnsmasq", "kvm",
        "nm-openvpn", "openvpn", "pki", "redis", "rtkit", "upower",
        "usbmux", "chrony", "nscd", "nslcd", "unbound"
    };

    if (username.isEmpty())
    {
        QMessageBox::critical(this, "Erro", "O nome de usuário está vazio.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (username.length() > 32)
    {
        QMessageBox::critical(this, "Erro", "O nome de usuário pode ter no máximo 32 caracteres.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (invalidUsernames.contains(username))
    {
        QMessageBox::critical(this, "Erro", "O nome de usuário <b>" + username + "</b> é reservado pelo sistema.", QMessageBox::StandardButton::Ok);
        return;
    }

    QRegularExpression regex("^[a-z_][a-z0-9_-]");
    if (!regex.match(username).hasMatch()) {
        QMessageBox::critical(this, "Erro", "Nome de usuário inválido. Use apenas letras minúsculas, números, '-' e '_'.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (isGroupNameUsed(username))
    {
        QMessageBox::critical(this, "Erro", "Já existe um grupo de usuários utilizando o nome \"" + username + "\". Por favor, escolha outro nome de usuário.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (rootPassword != rootPasswordConfirmLineEdit->text())
    {
        QMessageBox::critical(this, "Erro", "As senhas para root não conferem.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (userPassword != userPasswordConfirmLineEdit->text())
    {
        QMessageBox::critical(this, "Erro", "As senhas para o usuário " + username + " não conferem.", QMessageBox::StandardButton::Ok);
        return;
    }

    QMessageBox confirmationDialog;
    confirmationDialog.setWindowTitle("Confirme o seu usuário");
    grantUserAdministrativePrivileges->isChecked() ?
        confirmationDialog.setText("Criar usuário <b>" + username + "</b> <i>com privilégios administrativos</i>?")
        : confirmationDialog.setText("Criar usuário <b>" + username + "</b> <i>sem privilégios administrativos</i>?");

    confirmationDialog.setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::Cancel);

    if (confirmationDialog.exec() != QMessageBox::Ok)
    {
        return;
    } else {
        const QString& rootPassword = rootPasswordLineEdit->text();
        const QString& userPassword = userPasswordLineEdit->text();

        QString grantUserAdministrativePrivilegesCmd = "";

        QProcess* configureUsersProcess = new QProcess;
        QStringList arguments;
        arguments << "/mnt/new_root" << "/bin/bash" << "-c"
                  << "echo root:" + rootPassword + " | chpasswd; "
                     "useradd -m " + username + "; "
                     "echo " + username + ":" + userPassword + " | chpasswd; "
                     + (grantUserAdministrativePrivileges->isChecked() ? "usermod -aG wheel " + username + ";" : "")
                     + "exit;";

        configureUsersProcess->start("chroot", arguments);

        connect(configureUsersProcess, &QProcess::readyReadStandardOutput, this, [configureUsersProcess]() {
            qDebug() << configureUsersProcess->readAllStandardOutput();
        });
        
        connect(configureUsersProcess, &QProcess::readyReadStandardError, this, [configureUsersProcess]() {
            qDebug() << configureUsersProcess->readAllStandardError();
        });

        connect(configureUsersProcess, &QProcess::finished, this, [this, username](int exitCode, QProcess::ExitStatus processStatus){
            if (exitCode != 0 || processStatus == QProcess::ExitStatus::CrashExit)
            {
                QMessageBox::critical(this, "Erro", "Não foi possível configurar os usuários.", QMessageBox::Ok);
                return;
            } else {
                
                rootPasswordLineEdit->setEnabled(false);
                rootPasswordConfirmLineEdit->setEnabled(false);

                usernameLineEdit->setEnabled(false);
                userPasswordLineEdit->setEnabled(false);
                userPasswordConfirmLineEdit->setEnabled(false);
                grantUserAdministrativePrivileges->setEnabled(false);


                if (grantUserAdministrativePrivileges->isChecked())
                {
                    userLabel->setText("Usuário <b>" + username + "</b> criado com sucesso com privilégios administrativos");
                } else {
                    userLabel->setText("Usuário <b>" + username + "</b> criado com sucesso sem privilégios administrativos");
                }

                confirmButton->hide();
                page->setCanAdvance(true);
            }
        });

    }
}
