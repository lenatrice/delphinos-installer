#include "packagesPage.hpp"
#include <QRegularExpression>
#include <QDir>
#include <QApplication>

PackagesPage::PackagesPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Seleção de pacotes", "Escolha os pacotes que deseja instalar no sistema, e clique em \"Instalar\" para prosseguir", 480, 640, this
    );
    page->setCanAdvance(false);

    formLayout = new QFormLayout;
    QHBoxLayout* packageSelectionLayout = new QHBoxLayout;
    packageListWidget = new QListWidget;
    packageListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);

    for (QString package : basicPackages)
    {
        QListWidgetItem* item = new QListWidgetItem(package);
        item->setFlags(Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
    }

    for (QString package : optionalPackages)
    {
        QListWidgetItem* item = new QListWidgetItem(package);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
    }

    for (QString package : uncheckedPackages)
    {
        QListWidgetItem* item = new QListWidgetItem(package);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        packageListWidget->addItem(item);
    }
    
    packageSelectionButtonsLayout = new QVBoxLayout;
    packageSelectionButtonsLayout->setAlignment(Qt::AlignTop);
    installAllButton = new QPushButton("Instalar todos os pacotes");
    installBasicButton = new QPushButton("Instalar apenas pacotes básicos");
    installBasicAndOptionalButton = new QPushButton("Instalar pacotes básicos e opcionais");
    customInstallationButton = new QPushButton("Instalação personalizada");
    
    packageSelectionButtonsLayout->addWidget(installAllButton);
    packageSelectionButtonsLayout->addWidget(installBasicButton);
    packageSelectionButtonsLayout->addWidget(installBasicAndOptionalButton);
    packageSelectionButtonsLayout->addWidget(customInstallationButton);

    packageSelectionLayout->addWidget(packageListWidget, 1);
    packageSelectionLayout->addLayout(packageSelectionButtonsLayout);

    formLayout->addRow(new QLabel("Pacotes a serem instalados:"));
    formLayout->addRow(packageSelectionLayout);

    QButtonGroup* pageSelectionButtonGroup = new QButtonGroup(this);
    pageSelectionButtonGroup->addButton(installAllButton);
    pageSelectionButtonGroup->addButton(installBasicButton);
    pageSelectionButtonGroup->addButton(installBasicAndOptionalButton);
    pageSelectionButtonGroup->addButton(customInstallationButton);

    QPushButton* installSystemButton = new QPushButton("Instalar o sistema");
    packageSelectionButtonsLayout->addSpacing(300);
    packageSelectionButtonsLayout->addWidget(installSystemButton);
    
    installAllButton->setCheckable(true);
    installBasicButton->setCheckable(true);
    installBasicAndOptionalButton->setCheckable(true);
    customInstallationButton->setCheckable(true);

    // Default to "Install basic and optional packages"
    installBasicAndOptionalButton->setChecked(true);

    connect(installAllButton, &QPushButton::clicked, this, &PackagesPage::onInstallAllButtonClicked);
    connect(installBasicButton, &QPushButton::clicked, this, &PackagesPage::onInstallBasicButtonClicked);
    connect(installBasicAndOptionalButton, &QPushButton::clicked, this, &PackagesPage::onInstallBasicAndOptionalButtonClicked);

    connect(installSystemButton, &QPushButton::clicked, this, &PackagesPage::onInstallSystemButtonClicked);

    // Add video player for loading animation
    QVBoxLayout* installingLayout = new QVBoxLayout;
    installingLayout->setAlignment(Qt::AlignHCenter);
    installingAnimation = new LoadingAnimation;
    installingAnimation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    installingLabel = new QLabel("Instalando o sistema...");
    installingLabel->hide();
    installingLabel->setAlignment(Qt::AlignHCenter);

    installingLayout->addWidget(installingAnimation);

    formLayout->addRow(installingLayout);
    formLayout->addRow(installingLabel);

    // Initiate pacstrap process
    pacstrapProcess = new QProcess(this);

    // Add layout to the page
    page->addLayout(formLayout);
}


void PackagesPage::onPackageListChanged(QListWidgetItem* item)
{
    customInstallationButton->setChecked(true);
}

void PackagesPage::onInstallAllButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        packageListWidget->item(i)->setCheckState(Qt::Checked);
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);
}

void PackagesPage::onInstallBasicButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        QListWidgetItem* item = packageListWidget->item(i);
        if (basicPackages.contains(item->text()))
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);
}

void PackagesPage::onInstallBasicAndOptionalButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        QListWidgetItem* item = packageListWidget->item(i);
        if (basicPackages.contains(item->text()) || optionalPackages.contains(item->text()))
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);

}

void PackagesPage::onInstallSystemButtonClicked(bool checked)
{
    QStringList packages;
    for (int i = 0; i < packageListWidget->count(); ++i) {
        QListWidgetItem* item = packageListWidget->item(i);
        if (item->checkState() == Qt::Checked) packages << item->text();
    }

    QStringList arguments;
    arguments << "/mnt/new_root";
    arguments.append(packages);

    qDebug() << "Running pacstrap with arguments:" << arguments;

    pacstrapProcess = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    pacstrapProcess->setProcessEnvironment(env);

    QStringList pkexecArguments;
    pkexecArguments << "pacstrap" << arguments;

    pacstrapProcess->start("pkexec", pkexecArguments);

    bool installSpinnerHasStarted = false;

    connect(pacstrapProcess, &QProcess::readyRead, this, [this, installSpinnerHasStarted]() {

        if (!installSpinnerHasStarted)
        {
            installingAnimation->start();
            installingLabel->show();
        }

        qDebug() << pacstrapProcess->readAll();
    });

    connect(pacstrapProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit) {
            qDebug() << "pacstrap process crashed.";
        } else if (exitCode != 0) {
            qDebug() << "pacstrap finished with error exit code" << exitCode;
            QMessageBox::critical(nullptr, "Error", "There was an error installing the system.");
        } else {
            qDebug() << "pacstrap installation finished successfully!";
            QMessageBox::information(nullptr, "Success", "System installation completed successfully.");

            // Clean up the QProcess object
            pacstrapProcess->deleteLater();
        }
        installingAnimation->stop();
        installingLabel->hide();
    });
}