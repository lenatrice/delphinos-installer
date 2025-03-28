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

#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QUuid>
#include <QMetaType>

#pragma push_macro("signals")
#undef signals
#include <glib.h>
#pragma pop_macro("signals")

#pragma once

#ifndef NM_CONSTANTS

const QString NM_SERVICE = "org.freedesktop.NetworkManager";
const QString NM_PATH = "/org/freedesktop/NetworkManager";
const QString NM_SETTINGS = "org.freedesktop.NetworkManager.Settings";
const QString NM_SETTINGS_PATH = "/org/freedesktop/NetworkManager/Settings";
const QString NM_DEVICE_INTERFACE = "org.freedesktop.NetworkManager.Device";

#define NM_CONSTANTS 
#endif

using ConnectionSettingsItem = QMap<QString, QVariant>;
using ConnectionSettings = QMap<QString, ConnectionSettingsItem>;
using ConnectionSecretsItem = ConnectionSettingsItem;
using ConnectionSecrets = ConnectionSettings;

class WifiAccessPoint;

class NetworkDevice : public QObject
{
Q_OBJECT
public:
    const QDBusObjectPath dbusPath;
    const QString name;
    const QString type;
    uint state;

    void monitor();
    void stopMonitoring();
    QList<WifiAccessPoint*> accessPoints;
    explicit NetworkDevice(const QDBusObjectPath& _dbusPath, const QString& _name, const QString& _type) : dbusPath(_dbusPath), name(_name), type(_type), state(0)
    {
        monitor();
    }

    ~NetworkDevice()
    {
        stopMonitoring();
        qDeleteAll(accessPoints); 
    }

private slots:
    void onPropertiesChanged(const QString& interfaceName, const QVariantMap& changedProperties, const QStringList& invalidatedProperties);
};
Q_DECLARE_METATYPE(NetworkDevice*)

class WifiAccessPoint : public QObject
{
Q_OBJECT
public:
    const QDBusObjectPath dbusPath;
    const NetworkDevice* networkDevice;
    const QString ssid;
    bool isConnected;

    explicit WifiAccessPoint(const QDBusObjectPath& _dbusPath, const NetworkDevice* _networkDevice, QString _ssid) : dbusPath(_dbusPath), networkDevice(_networkDevice), ssid(_ssid) {};
};
Q_DECLARE_METATYPE(WifiAccessPoint*)

class NetworkConnection : public QObject
{
Q_OBJECT
public:
    void monitor();
    void stopMonitoring();

    const QDBusObjectPath dbusPath;
    const NetworkDevice* networkDevice;
    const WifiAccessPoint* accessPoint;
    uint state;
    
    explicit NetworkConnection(const QDBusObjectPath& _dbusPath, const NetworkDevice* _networkDevice, const WifiAccessPoint* _accessPoint = nullptr) : dbusPath(_dbusPath), networkDevice(_networkDevice), accessPoint(_accessPoint) {};

private slots:
    void onPropertiesChanged(const QString& interfaceName, const QVariantMap& changedProperties, const QStringList& invalidatedProperties);
};
Q_DECLARE_METATYPE(NetworkConnection*)
