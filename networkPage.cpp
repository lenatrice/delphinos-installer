#include "networkPage.hpp"
#include <QLineEdit>
#include <QInputDialog>
#include <QDBusMetaType>
#include <QDialog>
#include <QMap>

static const QMap<int, QString> networkTypeMap
{
    { 1, "Ethernet" },
    { 2, "Wi-Fi" },
    { 3, "Bridge" },
    { 4, "Loopback" },
    { 5, "Tunnel" },
    { 6, "VLAN" },
    { 7, "ADSL" },
    { 8, "MACVLAN" },
    { 10, "Bond" },
    { 11, "Team" },
    { 12, "IP Tunnel" },
    { 13, "Virtual Bridge" },
    { 14, "MACsec" },
    { 15, "Virtual Ethernet" },
    { 16, "Virtual Network" },
    { 17, "Point-to-Point Protocol" },
    { 18, "OVS Bridge" },
    { 19, "OVS Port" },
    { 20, "OVS Interface" },
    { 21, "Generic" },
    { 22, "TUN" },
    { 23, "TAP" },
    { 24, "IP6 Tunnel" },
    { 25, "WireGuard" },
    { 26, "Wi-Fi Point-to-Point" },
    { 27, "VRF" },
    { 28, "Lowpan" },
    { 29, "Dummy" },
    { 30, "Point-to-Point Protocol over Ethernet" },
    { 31, "InfiniBand" },
    { 32, "Loopback" },
};

NetworkPage::NetworkPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Configurar conexão de rede",
        "Selecione a rede que deseja utilizar para instalar o sistema.",
        480, 640, this
    );

    formLayout = new QFormLayout;
    
    deviceList = new QListWidget;
    formLayout->addRow("Dispositivos de rede:", deviceList);
    
    // Wi-Fi access points list
    wifiAccessPointList = new QListWidget;
    wifiAccessPointList->hide(); // Hide Wi-Fi access points list until a Wi-Fi network device is selected
    
    // Network function buttons
    functionButtons = new QHBoxLayout;
    functionButtons->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    
    refreshButton = new QPushButton("Atualizar");
    disconnectButton = new QPushButton("Desconectar");
    connectButton = new QPushButton("Conectar");
    functionButtons->addWidget(refreshButton);
    functionButtons->addWidget(disconnectButton);
    functionButtons->addWidget(connectButton);
    
    populateNetworkDevices();
    
    connect(deviceList, &QListWidget::currentItemChanged,
        [this](QListWidgetItem* networkDeviceItem)
        {
            updateNetworkDevice();
        }
    );
    
    connect(refreshButton, &QPushButton::clicked, this,
        [this](const bool)
        {
            populateNetworkDevices();
        }
    );
    
    connect(connectButton, &QPushButton::clicked, this,
        [this](const bool)
        {
            connectNetwork();
        }
    );
    
    page->addLayout(formLayout);
    page->addLayout(functionButtons);
}


void NetworkPage::populateNetworkDevices()
{
    QListWidgetItem* currentDevice = deviceList->currentItem();
    QListWidgetItem* currentAccessPoint = wifiAccessPointList->currentItem();
    QString currentDevicePath;

    if (currentDevice)
    {
        currentDevicePath = getNetworkDevice(currentDevice)->dbusPath.path();
    }

    qDebug() << "Clearing list of network devices";
    clearNetworkDevices(networkDevices, deviceList);

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

    if (!dbusInterface.isValid())
    {   
        qCritical() << "Failed to connect to D-Bus: " << QDBusConnection::systemBus().lastError().message();
        return;
    }

    QDBusReply<QList<QDBusObjectPath>> getAllDevicesReply = dbusInterface.call("GetAllDevices");
    QList<QDBusObjectPath> dbusDevices = getAllDevicesReply.value();

    if (!getAllDevicesReply.isValid())
    {
        qCritical() << "Failed to call GetAllDevices: " << QDBusConnection::systemBus().lastError().message();
        return;
    }
    
    qDebug() << "Populating network devices";
    for (const QDBusObjectPath& devicePath : dbusDevices)
    {
        QDBusInterface deviceInterface("org.freedesktop.NetworkManager", devicePath.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> deviceReply = deviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");

        if (!deviceReply.isValid())
        {
            qCritical() << "Failed to get device interface for path" << devicePath.path() << ":" << deviceReply.error().message();
            continue;
        }

        NetworkDevice* newNetworkDevice;
        QString deviceName = deviceReply.value().toString();
        QDBusReply<QVariant> deviceTypeReply = deviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "DeviceType");

        if (!deviceTypeReply.isValid())
        {
            qWarning() << "Coult not determine type of device" << deviceName << ":" << deviceTypeReply.error().message();
        }

        int deviceType = deviceTypeReply.value().toInt();
        newNetworkDevice = new NetworkDevice(devicePath, deviceName, networkTypeMap.value(deviceType, "Unknown"));
        networkDevices.push_back(newNetworkDevice);
        deviceName += " (" + networkTypeMap.value(deviceType, "Unknown") + ")";
        QListWidgetItem* newItem = new QListWidgetItem(deviceName);
        newItem->setData(networkDeviceObjRole, QVariant::fromValue(newNetworkDevice));
        deviceList->addItem(newItem);
    }

    // Reset previously selected item
    if (!currentDevicePath.isEmpty())
    {
        for (int i = 0; i < deviceList->count(); i++)
        {
            QListWidgetItem* item = deviceList->item(i);

            if (item->data(networkDeviceObjRole).value<NetworkDevice*>()->dbusPath.path() == currentDevicePath)
            {
                deviceList->setCurrentItem(item);
                break;
            }
        }
    }

    updateNetworkDevice();
}


void NetworkPage::updateNetworkDevice()
{
    qDebug() << "Updating network device";
    
    NetworkDevice* networkDevice = getNetworkDevice(deviceList->currentItem());

    for (int i = 0; i < formLayout->rowCount(); ++i)
    {
        QLayoutItem* item = formLayout->itemAt(i, QFormLayout::FieldRole);
        if (item && item->widget() == wifiAccessPointList) {
            formLayout->removeWidget(wifiAccessPointList);
            formLayout->removeRow(i);
            wifiAccessPointList->clear();
            wifiAccessPointList->hide();
            break;
        }
    }

    if (!networkDevice) {
        return; 
    }

    // If the selected device is Wi-Fi, generate access points
    if (networkDevice->type == "Wi-Fi")
    {
        qDebug() << "Creating new Wi-Fi interface for" << networkDevice->dbusPath.path();
        QDBusInterface wifiInterface("org.freedesktop.NetworkManager", networkDevice->dbusPath.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());

        formLayout->addRow("Pontos de acesso:", wifiAccessPointList);
        wifiAccessPointList->show();

        if (!wifiInterface.isValid())
        {
            qWarning() << "Failed to get Wi-Fi device interface for" << networkDevice->dbusPath.path() << ":" << wifiInterface.lastError().message();
            return;
        }

        QDBusReply<QList<QDBusObjectPath>> wifiAccessPointsReply = wifiInterface.call("GetAllAccessPoints");
        if (!wifiAccessPointsReply.isValid())
        {
            qWarning() << "Failed to call Wi-Fi device method GetAllAccessPoints:" << wifiAccessPointsReply.error().message();
            return;
        }

        for (const QDBusObjectPath& accessPointIterator : wifiAccessPointsReply.value())
        {
            const QDBusObjectPath& accessPointPath = accessPointIterator;
            QDBusInterface accessPointInterface("org.freedesktop.NetworkManager", accessPointIterator.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
            if (!accessPointInterface.isValid())
            {
                qWarning() << "Failed to get access point interface for path" << accessPointIterator.path() << ":" << accessPointInterface.lastError().message();
                continue;
            }

            QDBusReply<QVariant> accessPointSsidReply = accessPointInterface.call("Get", "org.freedesktop.NetworkManager.AccessPoint", "Ssid");
            if (!accessPointSsidReply.isValid())
            {
                qWarning() << "Failed to get access point SSID for path" << accessPointIterator.path() << ":" << accessPointSsidReply.error().message();
                continue;
            }

            WifiAccessPoint* newAccessPoint = new WifiAccessPoint(accessPointPath, networkDevice, accessPointSsidReply.value().toString());
            networkDevice->accessPoints.push_back(newAccessPoint);

            qDebug() << "Access point SSID:" << newAccessPoint->ssid;

            QListWidgetItem* newAccessPointItem = new QListWidgetItem(newAccessPoint->ssid);
            newAccessPointItem->setData(wifiAccessPointObjRole, QVariant::fromValue<WifiAccessPoint*>(newAccessPoint));
            wifiAccessPointList->addItem(newAccessPointItem);
        }
    }
}



void NetworkPage::connectNetwork()
{
    qDBusRegisterMetaType<ConnectionSettings>();
    qDBusRegisterMetaType<ConnectionSettingsItem>();

    QDBusInterface nmInterface(NM_SERVICE, NM_PATH, NM_SERVICE, QDBusConnection::systemBus());
    if (!nmInterface.isValid())
    {
        qCritical() << "Failed to connect to NetworkManager D-Bus service:" << nmInterface.lastError().message();
        return;
    }
    QDBusInterface nmSettingsInterface(NM_SERVICE, NM_SETTINGS_PATH, NM_SETTINGS, QDBusConnection::systemBus());
    if (!nmSettingsInterface.isValid())
    {
        qCritical() << "Failed to connect to NetworkManager settings D-Bus service:" << nmSettingsInterface.lastError().message();
        return;
    }

    const NetworkDevice* networkDevice = getNetworkDevice(deviceList->currentItem());
    WifiAccessPoint* wifiAccessPoint = getAccessPoint(wifiAccessPointList->currentItem());

    NetworkConnection* networkConnection = nullptr;

    if (networkDevice->type == "Wi-Fi")
    {
        if (wifiAccessPoint)
        {
            ConnectionSettings nmSettings;

            // Check for existing connections for this network
            QDBusReply<QList<QDBusObjectPath>> listConnectionsReply = nmSettingsInterface.call("ListConnections");
            QString existingPassword;
            if (!listConnectionsReply.isValid())
            {
                qWarning() << "Failed to get list connections:" << listConnectionsReply.error().message();
            }

            for (const QDBusObjectPath& tempConnectionPath : listConnectionsReply.value())
            {
                QDBusInterface connectionInterface(NM_SERVICE, tempConnectionPath.path(), "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
                if (!connectionInterface.isValid())
                {
                    qWarning() << "Failed to get connection interface:" << connectionInterface.lastError().message();
                    continue;
                }
                
                QDBusReply<ConnectionSettings> getSettingsReply = connectionInterface.call("GetSettings");
                if (!getSettingsReply.isValid())
                {
                    qWarning() << "Failed to get connection settings for " << tempConnectionPath.path() << ":" << getSettingsReply.error().message();
                    continue;
                }

                ConnectionSettings tempNmSettings = getSettingsReply.value();
                if (!tempNmSettings.contains("802-11-wireless"))
                {
                    continue;
                }

                QByteArray tempSsid = tempNmSettings["802-11-wireless"]["ssid"].toByteArray();
                //Check if found SSID corresponds to the selected Access Point
                if (tempSsid != QByteArray(wifiAccessPoint->ssid.toUtf8()))
                {
                    continue;
                }

                qDebug() << "Found existing connection for SSID:" << wifiAccessPoint->ssid;
                networkConnection = new NetworkConnection(tempConnectionPath, networkDevice, wifiAccessPoint);
                nmSettings = tempNmSettings;
                if (!nmSettings.contains("802-11-wireless-security"))
                {
                    continue;
                }

                existingPassword = nmSettings["802-11-wireless-security"]["psk"].toString();

                // If GetSettings retrieve no password, try GetSecrets
                if (existingPassword.isEmpty())
                {
                    QDBusReply<ConnectionSecrets> getSecretsReply = connectionInterface.call("GetSecrets", "802-11-wireless-security");
                    if (!getSecretsReply.isValid())
                    {
                        qWarning() << "Failed to get connection secrets:" << getSettingsReply.error().message();
                    }

                    ConnectionSecrets nmSecrets = getSecretsReply.value();
                    existingPassword = nmSecrets["802-11-wireless-security"]["psk"].toString();

                    for (const QString& key : nmSecrets.keys())
                    {
                        // If network settings doesn't contain the same secrets key, add the entire key
                        if (!nmSettings.contains(key))
                        {
                            nmSettings[key] = nmSecrets[key];
                            qDebug() << "Added secret key to settings:" << nmSettings[key];
                            continue;
                        }

                        // If network settings contain the same secrets key, change its subkeys
                        for (const QString& subkey : nmSecrets[key].keys())
                        {
                            nmSettings[key][subkey] = nmSecrets[key][subkey];
                            qDebug() << "Changed settings key from secret key:" << nmSettings[key];
                        }
                    }
                    nmSettings["802-11-wireless-security"]["psk-flags"] = 0;
                }

                qDebug() << "Existing password:" << existingPassword;
            }

            // If there is no existing connection for this access point, add a new one
            if (networkConnection == nullptr)
            {
                qDebug() << "Requesting access point password";

                QString wifiAccessPointPassword = requestAccessPointPassword(wifiAccessPoint->ssid);

                qDebug() << "Configuring connection settings";
    
                ConnectionSettings settings;
                settings["connection"] = ConnectionSettingsItem{
                    { "type", "802-11-wireless" },
                    { "uuid", QUuid::createUuid().toString().remove('{').remove('}')},
                    { "id", wifiAccessPoint->ssid }
                };
                settings["802-11-wireless"] = ConnectionSettingsItem{
                    { "ssid", QByteArray(wifiAccessPoint->ssid.toUtf8()) },
                    { "security", "802-11-wireless-security" },
                    { "mode", "infrastructure" }
                };
                settings["802-11-wireless-security"] = ConnectionSettingsItem{
                    { "key-mgmt", "wpa-psk" },
                    { "psk", wifiAccessPointPassword},
                    { "psk-flags", 0 }
                };
                settings["ipv4"] = ConnectionSettingsItem{
                    { "method", "auto"}
                };
                settings["ipv6"] = ConnectionSettingsItem{
                    { "method", "auto"}
                };

                qDebug() << "Using connection settings:";
                for (const QString& key : settings.keys())
                {
                    qDebug() << settings[key];
                }            

                QDBusReply<QDBusObjectPath> addConnectionReply = nmSettingsInterface.call("AddConnection", QVariant::fromValue(settings));

                if (addConnectionReply.isValid())
                {
                    networkConnection = new NetworkConnection(addConnectionReply.value(), networkDevice, wifiAccessPoint);
                    qDebug() << "Added new connection for SSID:" << wifiAccessPoint->ssid;
                } else {
                    qCritical() << "Failed to add connection:" << addConnectionReply.error().message();
                    return;
                }
            }

            if (networkConnection)
            {
                if (networkConnection->dbusPath.path().isEmpty())
                {
                    qCritical() << "Empty connection path";
                    return;
                }
            } else {
                qCritical() << "NetworkConnection object does not exist";
                return;
            }

            updateConnectionSettings(networkConnection->dbusPath, nmSettings);

            QDBusReply<QDBusObjectPath> activateConnectionReply = nmInterface.call("ActivateConnection",
                QVariant::fromValue(networkConnection->dbusPath),
                QVariant::fromValue(networkDevice->dbusPath),
                QVariant::fromValue(QDBusObjectPath("/"))
            );

            if (activateConnectionReply.isValid())
            {
                qDebug() << "Successfully initiated Wi-Fi connection";

            } else {
                qCritical() << "Could not activate connection:" << activateConnectionReply.error().message();
            }
        } else {
            qWarning() << "Error: did not select Wi-Fi access point to connect";
        }
    // If the selected network device is ethernet
    } else if (networkDevice->type == "Ethernet") {

        // List all connections
        QDBusReply<QList<QDBusObjectPath>> listConnectionsReply = nmSettingsInterface.call("ListConnections");
        if (!listConnectionsReply.isValid()) {
            qWarning() << "Failed to get list connections:" << listConnectionsReply.error().message();
            return;
        }

        ConnectionSettings nmSettings;
        nmSettings["connection"] = ConnectionSettingsItem{
            { "type", "802-3-ethernet" },
            { "uuid",  QUuid::createUuid().toString().remove('{').remove('}') },
            { "id", networkDevice->type }
        };
        nmSettings["ipv4"] = ConnectionSettingsItem{
            { "method", "auto" }
        };
        nmSettings["ipv6"] = ConnectionSettingsItem{
            { "method", "auto" }
        };

        
        QDBusReply<QDBusObjectPath> addConnectionReply = nmSettingsInterface.call("AddConnection", QVariant::fromValue(nmSettings));
        if (!addConnectionReply.isValid())
        {
            qCritical() << "Failed to add Ethernet connection:" << addConnectionReply.error().message();
            return;
        }

        NetworkConnection* newNetworkConnection = new NetworkConnection(addConnectionReply.value(), networkDevice);
        networkConnections.push_back(newNetworkConnection);

        QDBusReply<QDBusObjectPath> activateConnectionReply = nmInterface.call("ActivateConnection",
            QVariant::fromValue(newNetworkConnection->dbusPath),
            QVariant::fromValue(networkDevice->dbusPath),
            QVariant::fromValue(QDBusObjectPath("/"))
        );

        if (activateConnectionReply.isValid())
        {
            qDebug() << "Successfully initiated Ethernet connection";
        } else {
            qCritical() << "Could not activate Ethernet connection" << activateConnectionReply.error().message();
        }
    }
}

QString NetworkPage::requestAccessPointPassword(const QString& wifiAccessPoint)
{
    bool wifiAccessPointPasswordSent;

    QString wifiAccessPointPassword = QInputDialog::getText(
        nullptr,
        "Password for " + wifiAccessPoint,
        "Enter the password for access point " + wifiAccessPoint,
        QLineEdit::Password,
        "",
        &wifiAccessPointPasswordSent
    );

    if (!wifiAccessPointPasswordSent || wifiAccessPointPassword.isEmpty())
    {
        qDebug() << "Password input was cancelled or is empty";
        return QString();
    }

    return wifiAccessPointPassword;
}

void NetworkPage::updateConnectionSettings(QDBusObjectPath connectionPath, ConnectionSettings nmSettings)
{
    qDebug() << "Using connection settings:";
    for (const QString& key : nmSettings.keys())
    {
        qDebug() << nmSettings[key];
    }


    QDBusInterface connectionInterface(NM_SERVICE, connectionPath.path(), "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus());
    QDBusReply<void> updateSettingsReply = connectionInterface.call("Update", QVariant::fromValue(nmSettings));
    if (updateSettingsReply.isValid())
    {
        qDebug() << "Updated settings for existing connection";
    } else {
        qCritical() << "Failed to update connection settings:" << updateSettingsReply.error().message();
    }
    QDBusReply<void> saveReply = connectionInterface.call("Save");
    if (saveReply.isValid())
    {
        qDebug() << "Saved settings and secrets for existing connection";
    } else {
        qCritical() << "Failed to save connection settings:" << saveReply.error().message();
    }
}