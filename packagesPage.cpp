#include "packagesPage.hpp"
#include <QRegularExpression>
#include <QDir>

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

    QVBoxLayout* progressLayout = new QVBoxLayout;
    progressLayout->setAlignment(Qt::AlignBottom);
    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressLayout->addWidget(progressBar);
    formLayout->addRow(progressLayout);

    // Set up QProcess and its connections
    pacstrapProcess = new QProcess(this);

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

    // Set up pacstrap command and arguments
    QStringList arguments;
    arguments << "/mnt/new_root"; // Specify the root directory first
    arguments.append(packages); // Add the packages to install

    qDebug() << "Running pacstrap with arguments:" << arguments;

    pacstrapProcess = new QProcess(this);

    // Use pkexec to run pacstrap with elevated privileges
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    pacstrapProcess->setProcessEnvironment(env);

    QStringList pkexecArguments;
    pkexecArguments << "pacstrap" << arguments;

    pacstrapProcess->start("pkexec", pkexecArguments);

    connect(pacstrapProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        qDebug() << pacstrapProcess->readAllStandardOutput();
    });

    connect(pacstrapProcess, &QProcess::readyReadStandardError, this, [this]() {
        qDebug() << pacstrapProcess->readAllStandardError();
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
    });
}