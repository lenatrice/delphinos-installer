#include "installationPage.hpp"
#include <QRegularExpression>
#include <QDir>
#include <QApplication>

InstallationPage::InstallationPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Seleção de pacotes", "Escolha os pacotes que deseja instalar no sistema, e clique em \"Instalar\" para prosseguir", 760, 640, this
    );
    page->setCanAdvance(false);

    formLayout = new QFormLayout;
    QHBoxLayout* packageSelectionLayout = new QHBoxLayout;
    packageListWidget = new QListWidget;
    packageListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);

    // Add the packages to the list widget
    for (auto it = basicPackages.constBegin(); it != basicPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(packageDescription);
        item->setData(packageNameRole, it.key());
        item->setFlags(Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
        processLabels.insert(it.key(), it.value());
    }

    for (auto it = optionalPackages.constBegin(); it != optionalPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(packageDescription);
        item->setData(packageNameRole, it.key());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        packageListWidget->addItem(item);
        processLabels.insert(it.key(), it.value());
    }

    for (auto it = uncheckedPackages.constBegin(); it != uncheckedPackages.constEnd(); it++)
    {
        QString packageDescription = it.value();
        packageDescription[0] = packageDescription[0].toUpper();
        QListWidgetItem* item = new QListWidgetItem(packageDescription);
        item->setData(packageNameRole, it.key());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        packageListWidget->addItem(item);
        processLabels.insert(it.key(), it.value());
    }
    
    // Create package selection buttons
    packageSelectionButtonsLayout = new QVBoxLayout;
    packageSelectionButtonsLayout->setAlignment(Qt::AlignTop);
    installAllButton = new QPushButton("Instalar todos os pacotes");
    installBasicButton = new QPushButton("Instalar apenas pacotes básicos");
    installBasicAndOptionalButton = new QPushButton("Instalar pacotes básicos e opcionais");
    customInstallationButton = new QPushButton("Instalação personalizada");
    
    // Set the package selection buttons to be checkable
    installAllButton->setCheckable(true);
    installBasicButton->setCheckable(true);
    installBasicAndOptionalButton->setCheckable(true);
    customInstallationButton->setCheckable(true);
    
    // Add the package selection buttons to their layout
    packageSelectionButtonsLayout->addWidget(installAllButton);
    packageSelectionButtonsLayout->addWidget(installBasicButton);
    packageSelectionButtonsLayout->addWidget(installBasicAndOptionalButton);
    packageSelectionButtonsLayout->addWidget(customInstallationButton);

    // Group the package selection buttons 
    QButtonGroup* packageSelectionButtonGroup = new QButtonGroup(this);
    packageSelectionButtonGroup->addButton(installAllButton);
    packageSelectionButtonGroup->addButton(installBasicButton);
    packageSelectionButtonGroup->addButton(installBasicAndOptionalButton);
    packageSelectionButtonGroup->addButton(customInstallationButton);

    // Default selected packages to "Install basic and optional packages"
    installBasicAndOptionalButton->setChecked(true);

    // Add the package selection buttons to the package selection layout
    packageSelectionLayout->addWidget(packageListWidget, 1);
    packageSelectionLayout->addLayout(packageSelectionButtonsLayout);

    // Add package selection layout to the form layout
    formLayout->addRow(new QLabel("Pacotes a serem instalados:"));
    formLayout->addRow(packageSelectionLayout);

    // Create the install system button
    installSystemButton = new QPushButton("Instalar o sistema");
    packageSelectionButtonsLayout->addSpacing(300);
    packageSelectionButtonsLayout->addWidget(installSystemButton);

    // Connect the buttons clicked signals to their respective functions
    connect(installAllButton, &QPushButton::clicked, this, &InstallationPage::onInstallAllButtonClicked);
    connect(installBasicButton, &QPushButton::clicked, this, &InstallationPage::onInstallBasicButtonClicked);
    connect(installBasicAndOptionalButton, &QPushButton::clicked, this, &InstallationPage::onInstallBasicAndOptionalButtonClicked);

    connect(installSystemButton, &QPushButton::clicked, this, [this](bool checked)
    {
        installSystemButton->setEnabled(false);
        InstallationPage::onInstallSystemButtonClicked(checked);
        installSystemButton->setEnabled(true);
    });

    // Create the progress indicator for the installation 
    QVBoxLayout* installationProgressLayout = new QVBoxLayout;
    installationProgressLayout->setAlignment(Qt::AlignHCenter);

    installationProgressBar = new QProgressBar; // Progress bar
    installationProgressBar->hide();

    // Container to centralize statusIndicator + progressLabel
    QWidget* statusContainer = new QWidget;
    QHBoxLayout* installationProgressLabelLayout = new QHBoxLayout;
    installationProgressLabelLayout->setAlignment(Qt::AlignCenter);

    installationStatusIndicator = new StatusIndicator;
    installationProgressLabel = new QLabel(""); // Progress label
    installationProgressLabel->hide();

    installationProgressLabelLayout->addWidget(installationStatusIndicator);
    installationProgressLabelLayout->addWidget(installationProgressLabel);

    // Define o layout no container
    statusContainer->setLayout(installationProgressLabelLayout);

    // Adiciona a barra de progresso e o container de status ao layout principal
    installationProgressLayout->addWidget(installationProgressBar);
    installationProgressLayout->addWidget(statusContainer, 0, Qt::AlignHCenter);

    // Adiciona ao layout principal da página
    page->addLayout(formLayout);
    page->addLayout(installationProgressLayout);
}


void InstallationPage::onPackageListChanged(QListWidgetItem* item)
{
    customInstallationButton->setChecked(true);
}

void InstallationPage::onInstallAllButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        QListWidgetItem* item = packageListWidget->item(i);
        item->setCheckState(Qt::Checked);
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
}

void InstallationPage::onInstallBasicButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        QListWidgetItem* item = packageListWidget->item(i);
        QString packageName = item->data(packageNameRole).toString(); // Extract stored package name

        if (basicPackages.contains(packageName))
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
}

void InstallationPage::onInstallBasicAndOptionalButtonClicked(bool checked)
{
    disconnect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
    for (int i = 0; i < packageListWidget->count(); ++i)
    {
        QListWidgetItem* item = packageListWidget->item(i);
        QString packageName = item->data(packageNameRole).toString(); // Extract stored package name

        if (basicPackages.contains(packageName) || optionalPackages.contains(packageName))
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
    }
    connect(packageListWidget, &QListWidget::itemChanged, this, &InstallationPage::onPackageListChanged);
}

void InstallationPage::onInstallSystemButtonClicked(bool checked)
{
    QStringList installationScriptCommand;
    installationScriptCommand.append(QApplication::applicationDirPath() + "/systemInstallation/systemInstallation.sh");
    installationScriptCommand.append(getSelectedPackages());

    installationProcess = new QProcess;
    installationProcess->start("pkexec", QStringList() << "/bin/bash" << installationScriptCommand);

    connect(installationProcess, &QProcess::started, this, [this](){
        installationProgressBar->show();
        installationProgressBar->setRange(0, 0);
        installationProgressBar->setValue(0);

        installationStatusIndicator->setStatus(StatusIndicator::Warning);
        installationProgressLabel->setText("Aguardando senha");
        installationProgressLabel->show();
    });

    connect(installationProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        page->setCanAdvance(false);
        // If there is output from the process (e.g., progress updates from the script)
        QByteArray output = installationProcess->readAllStandardOutput();
        
        installationStatusIndicator->setStatus(StatusIndicator::Loading);
        
        QString outputStr = QString::fromUtf8(output);
        QStringList outputLines = outputStr.split("\n");
        outputLines.removeAll("");
        for (const QString& line : outputLines)
        {
            qDebug() << QString(line);
            if (line.contains("PROCEDURECOUNT:")) {
                QStringList parts = line.split(":");
                QString procedureCountStr = parts[1];
    
                bool procedureCountIsInt;
                int procedureCount = procedureCountStr.toInt(&procedureCountIsInt);
                if (procedureCountIsInt) {
                    installationProgressBar->setRange(0, procedureCount);
                } else {
                    qWarning() << "Failed to convert PROCEDURECOUNT to an integer";
                }
            }
            else if (line.contains("PROGRESS:")) {
                QStringList parts = line.split(":");
                QString progressStr = parts[1];
    
                bool currentProgressIsInt;
                int progress = progressStr.toInt(&currentProgressIsInt);
                if (currentProgressIsInt) {
                    installationProgressBar->setValue(progress);
                } else {
                    qWarning() << "Failed to convert PROGRESS to an integer";
                }
            }
            else if (line.contains("PREPARE NEW ROOT:")) {
                installationProgressLabel->setText("Preparando novo sistema de arquivos");
            }
            else if (line.contains("INSTALLING:")) {
                QStringList parts = line.split(":");
                QString packageName = parts[1];
    
                QString readableName;
                if (processLabels.contains(packageName)) {
                    readableName = processLabels[packageName];
                } else {
                    readableName = packageName;  // Default to the package name if not found in any map
                }
    
                installationProgressLabel->setText("Instalando " + readableName);
            }
            else if (line.contains("CONFIGURING:")) {
                QStringList parts = line.split(":");
                QString name = parts[1];
    
                QString readableName;
    
                if (processLabels.contains(name)) {
                    readableName = processLabels[name];
                } else {
                    readableName = name;
                }
    
                installationProgressBar->setValue(currentPackageIndex);
    
                installationStatusIndicator->setStatus(StatusIndicator::Loading);
                installationProgressLabel->setText("Configurando " + readableName);
            }
            else if (line.contains("ACTIVATING:")) {
                QStringList parts = line.split(":");
                QString name = parts[1];
    
                QString readableName;
    
                if (processLabels.contains(name)) {
                    readableName = processLabels[name];
                } else {
                    readableName = name;
                }
    
                installationProgressBar->setValue(currentPackageIndex);
    
                installationStatusIndicator->setStatus(StatusIndicator::Loading);
                installationProgressLabel->setText("Ativando serviço do sistema " + readableName);
            }
            else if (line.contains("ERROR:")) {
                QStringList parts = line.split(":");
                QString name = parts[1];
    
                QString readableName;
    
                if (processLabels.contains(name)) {
                    readableName = processLabels[name];
                } else {
                    readableName = name;
                }
    
                installationProgressBar->setValue(currentPackageIndex);
                installationErrorLabel = QString("Erro: " + readableName);
                return;
            }
        }
    });

    connect(installationProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        installationProgressBar->hide();
        installationProcess->deleteLater();
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            qDebug() << "System installation process failed";
            installationStatusIndicator->setStatus(StatusIndicator::Error);
            if (!installationErrorLabel.isEmpty())
            {
                installationProgressLabel->setText(installationErrorLabel);
            } else {
                installationProgressLabel->setText("Instalação do sistema falhou");
            }
        } else {
            qDebug() << "System installation finished successfully";
            installationStatusIndicator->setStatus(StatusIndicator::Ok);
            installationProgressLabel->setText("Instalação do sistema finalizada com sucesso");
            page->setCanAdvance(true);
        }
    });
}