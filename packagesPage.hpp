#include "mainWindow.hpp"
#include "loadingAnimation.hpp"
#include <QButtonGroup>
#include <QProcess>

#ifndef PACKAGESPAGE_H
#define PACKAGESPAGE_H

class PackagesPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;

    // Packages list
    QList<QString> basicPackages = {
        "base", "linux", "linux-firmware", "linux-headers", "grub", "efibootmgr", "networkmanager", "iwd", "plasma", "sddm", "sudo", "dolphin", "konsole", "bluez", "mesa", "kate"  
    };

    QList<QString> optionalPackages = {
         "htop", "nano", "vlc", "qbittorrent", "firefox", "spectacle", "wine", "obs-studio", "flatpak", "krita", "neofetch"
    };

    QList<QString> uncheckedPackages = {
        "base-devel", "ungoogled-chromium", "vim", "vscode", "nvidia", "code", "timeshift"
    };

    QListWidget* packageListWidget;
    bool manualChange = false;

    QPushButton* selectAllPackagesButton;

    QVBoxLayout* packageSelectionButtonsLayout;
    QPushButton* installAllButton;
    QPushButton* installBasicButton;
    QPushButton* installBasicAndOptionalButton;
    QPushButton* customInstallationButton;

    QProcess* pacstrapProcess;
    LoadingAnimation* installingAnimation;
    QLabel* installingLabel;

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
    };

    explicit PackagesPage(QWidget* parent);
};

#endif