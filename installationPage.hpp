#include "mainWindow.hpp"
#include "statusIndicator.hpp"
#include <QButtonGroup>
#include <QProcess>
#include <QProgressBar>

#ifndef InstallationPage_H
#define InstallationPage_H

class InstallationPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;

    // Packages list
    QMap<QString, QString> basicPackages = QMap<QString, QString>{
        { "base", "basic system packages" },
        { "linux", "Linux kernel" },
        { "linux-firmware", "Firmware for hardware devices" },
        { "linux-headers", "Headers for building modules" },
        { "grub", "bootloader" },
        { "efibootmgr", "EFI boot manager" },
        { "networkmanager", "network management service" },
        { "iwd", "Wi-Fi daemon" },
        { "plasma", "Plasma desktop environment" },
        { "sddm", "Login and display manager" },
        { "sudo", "privilege elevation support" },
        { "mesa", "open-source graphics driver" },
        { "bluez", "Bluetooth protocol stack" }
    };

    QMap<QString, QString> optionalPackages = QMap<QString, QString>{
        { "ufw", "firewall configuration tool" },
        { "amd-ucode", "AMD CPU microcode updates" },
        { "intel-ucode", "Intel CPU microcode updates" },
        { "os-prober", "operational system prober" },
        { "zsh", "Z shell" },
        { "grml-zsh-config", "preconfigured Zsh setup from GRML" },
        { "htop", "command-line process viewer" },
        { "vim", "command-line text editor" },
        { "nano", "command-line text editor" },
        { "konsole", "console interface" },
        { "firefox", "web browser" },
        { "dolphin", "file explorer" },
        { "kate", "advanced text editor" },
        { "vlc", "media player supporting most formats" },
        { "qbittorrent", "torrent client" },
        { "spectacle", "screenshot tool" },
        { "wine", "Windows compatibility layer" },
        { "wine-mono", "Wine's implementation of Microsoft's .NET Framework" },
        { "flatpak", "Flatpak package manager" },
        { "neofetch", "commandline system info tool" },
        { "ntfs-3g", "NTFS filesystem driver" }
    };

    QMap<QString, QString> uncheckedPackages = QMap<QString, QString>{
        { "base-devel", "basic development tools" },
        { "code", "open-source build of Visual Studio Code" },
        { "nvidia", "NVIDIA proprietary GPU driver" },
        { "amdvlk", "AMD Vulkan driver" },
        { "vkd3d", "Direct3D 12 to Vulkan translation library" },
        { "mangohud", "Vulkan overlay for monitoring FPS and hardware stats" },
        { "krita", "digital painting and illustration software" },
        { "thunderbird", "Email client from Mozilla" },
        { "timeshift", "system restore utility" },
        { "kdenlive", "video editor" },
        { "virt-manager", "graphical manager for virtual machines (KVM/QEMU)" },
        { "retroarch", "multi-system emulator frontend" },
        { "steam", "Steam (Valve's game distribution platform)" },
        { "discord", "Discord voice, video and text chat" },
        { "telegram-desktop", "Telegram messenger" },
        { "obs-studio", "Screen recording and live streaming software" }
    };

    int packageNameRole = Qt::UserRole;

    QListWidget* packageListWidget;

    QString getPackageName(const QListWidgetItem* package)
    {
        return package->data(packageNameRole).value<QString>();
    };

    QStringList getSelectedPackages()
    {
        QStringList selectedPackages;
        for (int i = 0; i < packageListWidget->count(); i++)
        {
            QListWidgetItem* item = packageListWidget->item(i);
            
            if (item->checkState() == Qt::Checked)
            {
                selectedPackages.append(item->data(packageNameRole).value<QString>());
            }
        }
        return selectedPackages;
    }

    QPushButton* selectAllPackagesButton;

    QVBoxLayout* packageSelectionButtonsLayout;
    QPushButton* installAllButton;
    QPushButton* installBasicButton;
    QPushButton* installBasicAndOptionalButton;
    QPushButton* customInstallationButton;

    QProcess* installationProcess = nullptr;
    QProgressBar* installationProgressBar;
    StatusIndicator* installationStatusIndicator;
    QLabel* installationProgressLabel;
    int currentPackageIndex = 0;

private slots:
    void onPackageListChanged(QListWidgetItem *item);

    void onInstallAllButtonClicked(bool check);
    void onInstallBasicButtonClicked(bool check);
    void onInstallBasicAndOptionalButtonClicked(bool check);
    void onInstallSystemButtonClicked(bool check);

public: 
    PageContent* getPage()
    {
        return page;
    }

    explicit InstallationPage(QWidget* parent);

    /*~InstallationPage()
    {
        if (installationProcess)
        {   
            qDebug() << "Installation process exists with state" << installationProcess->state() << ", disconnecting signals";
            disconnect(installationProcess, nullptr, this, nullptr); 
            if (installationProcess->state() != QProcess::NotRunning)
            {
                qDebug() << "Installation process is running, killing the process";
                installationProcess->kill();
                if (!installationProcess->waitForFinished(30000))
                {
                    qCritical() << "ERROR: Installation script was not killed, trying terminate.";
                    installationProcess->terminate();
                    if (!installationProcess->waitForFinished(30000))
                    {
                        qCritical() << "ERROR: Installation script was not terminating.";
                        installationProcess->terminate();
                    }
                }
            }
            installationProcess->deleteLater();
        }
    }*/
};

#endif