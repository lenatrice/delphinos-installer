#include "mainWindow.hpp"
#include "network.hpp"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QUuid>
#include <QInputDialog>
#include <QDBusMetaType>

QWidget* MainWindow::pageCreateNetwork()
{
    WindowPage* page = new WindowPage(
        "Configurar conexão de rede",
        "Selecione a rede que deseja utilizar para instalar o sistema."
    );

    QWidget* networkWidget = new QWidget(this);
    QVBoxLayout* networkVBoxLayout = new QVBoxLayout(networkWidget);

    QFormLayout* networkFormLayout = new QFormLayout;
    networkFormLayout->setAlignment(Qt::AlignTop);
    networkVBoxLayout->addLayout(networkFormLayout);

    QListWidget* networkDeviceList = new QListWidget(this);
    networkDeviceList->setMaximumHeight(120);
    networkFormLayout->addRow("Dispositivos de rede:", networkDeviceList);

    // Wi-Fi access points list
    QListWidget* wifiAccessPointList = new QListWidget(this);
    wifiAccessPointList->hide(); // Hide Wi-Fi access points list until a Wi-Fi network device is selected

    // Network function buttons
    QHBoxLayout* networkFunctionButtons = new QHBoxLayout;
    networkFunctionButtons->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QPushButton* refreshNetworkDevicesButton = new QPushButton("Atualizar", this);
    QPushButton* disconnectButton = new QPushButton("Desconectar", this);
    QPushButton* connectButton = new QPushButton("Conectar", this);
    networkFunctionButtons->addWidget(refreshNetworkDevicesButton);
    networkFunctionButtons->addWidget(disconnectButton);
    networkFunctionButtons->addWidget(connectButton);

    networkVBoxLayout->addLayout(networkFunctionButtons);

    populateNetworkDevices(networkFormLayout, networkDeviceList, wifiAccessPointList);

    connect(networkDeviceList, &QListWidget::currentItemChanged,
        [this, &networkFormLayout, &wifiAccessPointList](QListWidgetItem* networkDeviceItem)
        {
            updateNetworkDevice(networkFormLayout, networkDeviceItem, wifiAccessPointList);
        }
    );

    connect(refreshNetworkDevicesButton, &QPushButton::clicked, this,
        [this, &networkFormLayout, &networkDeviceList, &wifiAccessPointList](const bool)
        {
            populateNetworkDevices(networkFormLayout, networkDeviceList, wifiAccessPointList);
        }
    );

    connect(connectButton, &QPushButton::clicked, this,
        [this, &networkFormLayout, &networkDeviceList, &wifiAccessPointList](const bool)
        {
            connectNetwork(networkDeviceList->currentItem(), wifiAccessPointList->currentItem());
        }
    );

    page->addWidget(networkWidget);

    return page;
}

void MainWindow::connectNetwork(QListWidgetItem* networkDeviceItem, QListWidgetItem* wifiAccessPointItem)
{
    const int networkDeviceType = networkDeviceItem->data(networkDeviceTypeRole).toInt();
    QDBusObjectPath networkDevicePath = QDBusObjectPath(networkDeviceItem->data(networkDevicePathRole).toString());

    const QString NM_SERVICE = "org.freedesktop.NetworkManager";
    const QString NM_PATH = "/org/freedesktop/NetworkManager";
    const QString NM_SETTINGS = "org.freedesktop.NetworkManager.Settings";
    const QString NM_SETTINGS_PATH = "/org/freedesktop/NetworkManager/Settings";

    qDBusRegisterMetaType<QMap<QString, QVariant>>();
    qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant>>>();

    if (networkTypeMap.value(networkDeviceType) == "Wi-Fi")
    {
        if (wifiAccessPointItem)
        {
            QDBusInterface nmInterface(NM_SERVICE, NM_PATH, NM_SERVICE, QDBusConnection::systemBus());

            QDBusInterface nmSettingsInterface(NM_SERVICE, NM_SETTINGS_PATH, NM_SETTINGS, QDBusConnection::systemBus());

            if (!nmInterface.isValid())
            {
                qCritical() << "Failed to connect to NetworkManager D-Bus service:" << nmInterface.lastError().message();
                return;
            }

            if (!nmSettingsInterface.isValid())
            {
                qCritical() << "Failed to connect to NetworkManager settings D-Bus service:" << nmSettingsInterface.lastError().message();
                return;
            }

            const QString wifiAccessPointPath = wifiAccessPointItem->data(wifiAccessPointPathRole).toString();

            bool wifiAccessPointPasswordSent;
            QString wifiAccessPointPassword = QInputDialog::getText(
                nullptr,
                "Password for " + wifiAccessPointItem->text(),
                "Enter the password for access point " + wifiAccessPointItem->text(),
                QLineEdit::Password,
                "",
                &wifiAccessPointPasswordSent
            );

            if (!wifiAccessPointPasswordSent || wifiAccessPointPassword.isEmpty())
            {
                qDebug() << "Password input was cancelled or is empty.";
                return;
            }

            QMap<QString, QMap<QString, QVariant>> settings;
            settings["connection"] = QMap<QString, QVariant>{
                { "type", "802-11-wireless" },
                { "uuid", QUuid::createUuid().toString().remove('{').remove('}')},
                { "id", wifiAccessPointItem->text() }
            };
            settings["802-11-wireless"] = QMap<QString, QVariant>{
                { "ssid", QByteArray(wifiAccessPointItem->text().toUtf8()) },
                { "mode", "infrastructure" }
            };
            settings["802-11-wireless-security"] = QMap<QString, QVariant>{
                { "key-mgmt", "wpa-psk"},
                { "psk", wifiAccessPointPassword }
            };
            settings["ipv4"] = QMap<QString, QVariant>{
                { "method", "auto"}
            };
            settings["ipv6"] = QMap<QString, QVariant>{
                { "method", "auto"}
            };

            QDBusReply<QDBusObjectPath> addConnectionReply = nmSettingsInterface.call("AddConnection", QVariant::fromValue(settings));

            if (addConnectionReply.isValid())
            {
                QDBusObjectPath connectionPath = addConnectionReply.value();

                QDBusReply<QDBusObjectPath> activateConnectionReply = nmInterface.call("ActivateConnection", QVariant::fromValue(connectionPath), QVariant::fromValue(networkDevicePath), QVariant::fromValue(QDBusObjectPath("/")));
                if (activateConnectionReply.isValid())
                {
                    qDebug() << "Successfully connected network.";
                } else {
                    qCritical() << "Could not connect to network:" << activateConnectionReply.error().message();
                }
            } else {
                qCritical() << "Failed to add connection:" << addConnectionReply.error().message();
            }

        } else {
            qCritical() << "Error: did not select Wi-Fi access point to connect";
        }
    } else {
        qDebug() << "Selected non Wi-Fi network device";
    }
}

void MainWindow::populateNetworkDevices(QFormLayout* networkFormLayout, QListWidget* networkDeviceList, QListWidget* wifiAccessPointList)
{
    QListWidgetItem* currentDevice = networkDeviceList->currentItem();
    QString currentDevicePath;
    if (currentDevice)
    {
        currentDevicePath = currentDevice->data(networkDevicePathRole).toString();
    }

    networkDeviceList->clear();

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

    if (dbusInterface.isValid())
    {
        QDBusReply<QList<QDBusObjectPath>> dbusReply = dbusInterface.call("GetAllDevices");
        QList<QDBusObjectPath> dbusDevices = dbusReply.value();

        if (dbusReply.isValid())
        {
            for (const QDBusObjectPath& devicePath : dbusDevices)
            {
                QDBusInterface deviceInterface("org.freedesktop.NetworkManager", devicePath.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                QDBusReply<QVariant> deviceReply = deviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
                if (deviceReply.isValid())
                {
                    QString deviceName = deviceReply.value().toString();
                    QDBusReply<QVariant> deviceTypeReply = deviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "DeviceType");
                    if (deviceTypeReply.isValid())
                    {
                        int deviceType = deviceTypeReply.value().toInt();
                        deviceName += " (" + networkTypeMap.value(deviceType, "Unknown") + ")";
                    } else {
                        qWarning() << "Coult not determine type of device" << deviceName << ":" << deviceTypeReply.error().message();
                    }
        
                    QListWidgetItem* newDevice = new QListWidgetItem(deviceName);
                    newDevice->setData(networkDevicePathRole, devicePath.path());
                    newDevice->setData(networkDeviceTypeRole, deviceTypeReply.value().toInt());
                    networkDeviceList->addItem(newDevice);
                } else {
                    qCritical() << "Failed to get device interface for path" << devicePath.path() << ":" << deviceReply.error().message();
                }
            }        
        } else{
            qCritical() << "Failed to call D-Bus method: " << dbusReply.error().message();
            return;
        }
    } else {
        qCritical() << "Failed to connect to D-Bus: " << QDBusConnection::systemBus().lastError().message();
        return;
    }

    // Reset previously selected item
    if (!currentDevicePath.isEmpty())
    {
        for (int i = 0; i < networkDeviceList->count(); i++)
        {
            QListWidgetItem* item = networkDeviceList->item(i);

            if (item->data(networkDevicePathRole) == currentDevicePath)
            {
                networkDeviceList->setCurrentItem(item);
                break;
            }
        }
    }

    updateNetworkDevice(networkFormLayout, networkDeviceList->currentItem(), wifiAccessPointList);
}


void MainWindow::updateNetworkDevice(QFormLayout* networkFormLayout, QListWidgetItem* networkDeviceItem, QListWidget* wifiAccessPointList)
{
    networkFormLayout->parentWidget()->setUpdatesEnabled(false);
    if (!networkDeviceItem) {
        for (int i = 0; i < networkFormLayout->rowCount(); ++i)
        {
            QLayoutItem* item = networkFormLayout->itemAt(i, QFormLayout::FieldRole);
            if (item && item->widget() == wifiAccessPointList) {
                networkFormLayout->removeWidget(wifiAccessPointList);
                networkFormLayout->removeRow(i);
                wifiAccessPointList->clear();
                wifiAccessPointList->hide();
                break;
            }
        }
        networkFormLayout->parentWidget()->setUpdatesEnabled(true);
        return; 
    }
    const QString networkDevicePath = networkDeviceItem->data(networkDevicePathRole).toString();
    const int networkDeviceType = networkDeviceItem->data(networkDeviceTypeRole).toInt();

    for (int i = 0; i < networkFormLayout->rowCount(); ++i)
    {
        QLayoutItem* item = networkFormLayout->itemAt(i, QFormLayout::FieldRole);
        if (item && item->widget() == wifiAccessPointList) {
            networkFormLayout->removeWidget(wifiAccessPointList);
            networkFormLayout->removeRow(i);
            wifiAccessPointList->clear();
            wifiAccessPointList->hide();
            break;
        }
    }

    if (networkDeviceType == networkTypeMap.key("Wi-Fi"))
    {
        QDBusInterface wifiInterface("org.freedesktop.NetworkManager", networkDevicePath, "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());

        networkFormLayout->addRow("Pontos de acesso:", wifiAccessPointList);
        wifiAccessPointList->show();

        if (wifiInterface.isValid())
        {
            QDBusReply<QList<QDBusObjectPath>> wifiAccessPointsReply = wifiInterface.call("GetAllAccessPoints");
            if (wifiAccessPointsReply.isValid())
            {
                for (const QDBusObjectPath& accessPointIterator : wifiAccessPointsReply.value())
                {
                    QString accessPointPath = accessPointIterator.path();
                    QDBusInterface accessPointInterface("org.freedesktop.NetworkManager", accessPointIterator.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
                    if (accessPointInterface.isValid())
                    {
                        QDBusReply<QVariant> accessPointNameReply = accessPointInterface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Ssid");
                        if (accessPointNameReply.isValid())
                        {
                            QListWidgetItem* newAccessPoint = new QListWidgetItem(accessPointNameReply.value().toString());
                            newAccessPoint->setData(wifiAccessPointPathRole, accessPointIterator.path());
                            wifiAccessPointList->addItem(newAccessPoint);
                        } else {
                            qWarning() << "Failed to get access point SSID for path" << accessPointIterator.path() << ":" << accessPointNameReply.error().message();
                        }
                    } else {
                        qWarning() << "Failed to get access point interface for path" << accessPointIterator.path() << ":" << accessPointInterface.lastError().message();
                    }
                }
            } else {
                qWarning() << "Failed to call Wi-Fi device method GetAllAccessPoints:" << wifiAccessPointsReply.error().message();
            }
        } else { 
            qWarning() << "Failed to get Wi-Fi device interface for" << networkDevicePath << ":" << wifiInterface.lastError().message();
        }
    } else {
    }

    networkFormLayout->parentWidget()->setUpdatesEnabled(true);
}
