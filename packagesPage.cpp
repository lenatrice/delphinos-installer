#include "packagesPage.hpp"
#include <QRegularExpression>
#include <QDir>
#include <QApplication>

PackagesPage::PackagesPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Seleção de pacotes", "Escolha os pacotes que deseja instalar no sistema, e clique em \"Instalar\" para prosseguir", 760, 640, this
    );
    page->setCanAdvance(false);

    formLayout = new QFormLayout;
    QHBoxLayout* packageSelectionLayout = new QHBoxLayout;
    packageListWidget = new QListWidget;
    packageListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(packageListWidget, &QListWidget::itemChanged, this, &PackagesPage::onPackageListChanged);

    for (auto it = basicPackages.constBegin(); it != basicPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(it.key() + " - " + packageDescription);
        item->setData(packageNameRole, it.key());
        item->setFlags(Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
    }

    for (auto it = optionalPackages.constBegin(); it != optionalPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(it.key() + " - " + packageDescription);
        item->setData(packageNameRole, it.key());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
    }

    for (auto it = uncheckedPackages.constBegin(); it != uncheckedPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(it.key() + " - " + packageDescription);
        item->setData(packageNameRole, it.key());
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
    installationProgress = new QProgressBar;
    installationProgress->hide();

    formLayout->addRow(installationProgress);

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
    QStringList installationScriptCommand;
    installationScriptCommand.append(QApplication::applicationDirPath() + "/systemInstallScript.sh");
    installationScriptCommand.append(getSelectedPackages());

    qDebug() << "Running:" << installationScriptCommand;

    installationProcess = new QProcess(this);
    installationProcess->start("pkexec", QStringList() << "/bin/bash" << installationScriptCommand);

    int totalPackages = getSelectedPackages().size();

    connect(installationProcess, &QProcess::started, this, [this, totalPackages](){
        installationProgress->show();
        installationProgress->setRange(0, totalPackages);
        installationProgress->setValue(0);
        installationProgress->setTextVisible(true);
        installationProgress->setFormat("Instalando");
    });

    connect(installationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        // If there is output from the process (e.g., progress updates from the script)
        QByteArray output = installationProcess->readAllStandardOutput();
        
        // For simplicity, we assume that the output contains the current package being installed
        // You should adapt this part based on the actual output of your script
        QString outputStr = QString::fromUtf8(output);
        if (outputStr.contains("INSTALLING:")) {
            currentPackageIndex++;
            installationProgress->setValue(currentPackageIndex);

            QStringList parts = outputStr.split(":");
            QString pkgName = parts[1];

            QString readableName;
            if (basicPackages.contains(pkgName)) {
                readableName = basicPackages[pkgName];
            } else if (optionalPackages.contains(pkgName)) {
                readableName = optionalPackages[pkgName];
            } else if (uncheckedPackages.contains(pkgName)) {
                readableName = uncheckedPackages[pkgName];
            } else {
                readableName = pkgName;  // Default to the package name if not found in any map
            }

            installationProgress->setFormat("Instalando " + readableName);
        }
    });

    connect(installationProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit) {
            qDebug() << "Installation process crashed.";
        } else if (exitCode != 0) {
            qDebug() << "Installation finished with error exit code" << exitCode;
            QMessageBox::critical(nullptr, "Error", "There was an error installing the system.");
        } else {
            qDebug() << "Installation finished successfully!";
            QMessageBox::information(nullptr, "Success", "System installation completed successfully.");
        }
        installationProgress->hide();
        installationProcess->deleteLater();
    });
}