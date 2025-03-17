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
    rootLabelIndicator = new StatusIndicator;
    rootLabelIndicator->hide();
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
    userLabelIndicator = new StatusIndicator;
    userLabelIndicator->hide();
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
        QMessageBox::critical(this, "Erro", "Seu nome de usuário está vazio.", QMessageBox::StandardButton::Ok);
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

    QRegularExpression regex("^[a-z_][a-z0-9_-]{0,31}$");
    if (!regex.match(username).hasMatch()) {
        QMessageBox::critical(this, "Erro", "Nome de usuário inválido. Use apenas letras minúsculas, números, '-', '_' e '.', e siga as regras de nomeação.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (isGroupNameUsed(username))
    {

    }

    if (rootPasswordLineEdit->text() != rootPasswordConfirmLineEdit->text())
    {
        QMessageBox::critical(this, "Erro", "As senhas para root não conferem.", QMessageBox::StandardButton::Ok);
        return;
    }

    if (userPasswordLineEdit->text() != userPasswordConfirmLineEdit->text())
    {
        QMessageBox::critical(this, "Erro", "As senhas para o usuário " + usernameLineEdit->text() + " não conferem.", QMessageBox::StandardButton::Ok);
        return;
    }

    QPointer<QMessageBox> confirmationDialog = new QMessageBox;
    confirmationDialog->setWindowTitle("Confirme o seu usuário");
    grantUserAdministrativePrivileges->isChecked() ?
        confirmationDialog->setText("Criar usuário <b>" + username + "</b> <i>com privilégios administrativos</i>?")
        : confirmationDialog->setText("Criar usuário <b>" + username + "</b> <i>sem privilégios administrativos</i>?");

    confirmationDialog->setStandardButtons(QMessageBox::StandardButton::Ok | QMessageBox::Cancel);

    if (confirmationDialog->exec() != QMessageBox::Ok)
    {
        return;
    } else {
        const QString& rootPassword = rootPasswordLineEdit->text();
        const QString& userPassword = userPasswordLineEdit->text();

        QString grantUserAdministrativePrivilegesCmd = "";

        QProcess* configureUsersProcess = new QProcess;
        QStringList arguments;
        arguments << "chroot" << "/mnt/new_root" << "/bin/bash" << "-c"
                  << "echo root:" + rootPassword + " | chpasswd; "
                     "useradd -m " + username + "; "
                     "echo " + username + ":" + userPassword + " | chpasswd; "
                     + (grantUserAdministrativePrivileges->isChecked() ? "usermod -aG wheel " + username + ";" : "");

        configureUsersProcess->start("pkexec", arguments);

        connect(configureUsersProcess, &QProcess::readyReadStandardOutput, this, [configureUsersProcess]() {
            qDebug() << configureUsersProcess->readAllStandardOutput();
        });
        
        connect(configureUsersProcess, &QProcess::readyReadStandardError, this, [configureUsersProcess]() {
            qDebug() << configureUsersProcess->readAllStandardError();
        });

        connect(configureUsersProcess, &QProcess::finished, this, [this, username](int exitCode, QProcess::ExitStatus processStatus){
            rootLabelIndicator->show();
            userLabelIndicator->show();
            if (exitCode != 0 || processStatus == QProcess::ExitStatus::CrashExit)
            {
                QMessageBox::critical(this, "Erro", "Não foi possível configurar os usuários.", QMessageBox::Ok);
                rootLabelIndicator->setStatus(StatusIndicator::Error);
                userLabelIndicator->setStatus(StatusIndicator::Error);
                return;
            } else {
                rootFormLayout->removeRow(rootPasswordLineEdit);
                rootFormLayout->removeRow(rootPasswordConfirmLineEdit);
                page->removeLayout(rootPasswordsMatchLayout);
                userFormLayout->removeRow(userPasswordLineEdit);
                userFormLayout->removeRow(userPasswordConfirmLineEdit);
                page->removeLayout(userPasswordsMatchLayout);

                rootLabelIndicator->setStatus(StatusIndicator::Ok);;
                rootLabel->setText("Usuário <b>root</b> configurado com sucesso");

                userLabelIndicator->setStatus(StatusIndicator::Ok);
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