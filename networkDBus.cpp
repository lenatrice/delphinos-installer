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

#include "networkDBus.hpp"

void NetworkDevice::monitor()
{
    bool connectedDBus = QDBusConnection::systemBus().connect(
        NM_SERVICE, dbusPath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
    );

    if (connectedDBus)
    {
        qDebug() << "Monitoring device state for" << dbusPath.path();
    } else {
        qCritical() << "Failed to connect to D-Bus signal for device" << dbusPath.path();
    }
}

void NetworkDevice::stopMonitoring()
{
    bool disconnectedDBus = QDBusConnection::systemBus().disconnect(
        NM_SERVICE, dbusPath.path(), "org.freedesktop.DBus.Properties",
        "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
    );
    if (disconnectedDBus) 
    {
        qDebug() << "Stopped monitoring connection state for" << dbusPath.path();
    } else {
        qCritical() << "Failed to disconnect D-Bus signal for" << dbusPath.path();
    }
}



void NetworkConnection::monitor()
{
    bool connectedDBus = QDBusConnection::systemBus().connect(NM_SERVICE, dbusPath.path(), "org.freedesktop.DBus.Properties",
        "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
    );

    if (connectedDBus)
    {
    }

    qDebug() << "Monitoring connection state continuously...";
}


void NetworkConnection::stopMonitoring()
{
    bool disconnectedDBus = QDBusConnection::systemBus().disconnect(
        NM_SERVICE, dbusPath.path(), "org.freedesktop.DBus.Properties",
        "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
    );
    if (disconnectedDBus) 
    {
        qDebug() << "Stopped monitoring connection state for" << dbusPath.path();
    } else {
        qCritical() << "Failed to disconnect D-Bus signal for" << dbusPath.path();
    }
}


void NetworkDevice::onPropertiesChanged(const QString& interfaceName, const QVariantMap& changedProperties, const QStringList& invalidatedProperties)
{
    if (interfaceName == "org.freedesktop.NetworkManager.Device")
    {
        if (changedProperties.contains("State"))
        {
            uint state = changedProperties["State"].toUInt();
            switch (state)
            {
                case 0:  // Unknown
                    qDebug() << "Device state: Unknown";
                    break;
                case 10: // Unmanaged
                    qDebug() << "Device state: Unmanaged";
                    break;
                case 20: // Unavailable
                    qDebug() << "Device state: Unavailable";
                    break;
                case 30: // Disconnected
                    qDebug() << "Device state: Disconnected";
                    break;
                case 40: // Prepare
                    qDebug() << "Device state: Prepare";
                    break;
                case 50: // Config
                    qDebug() << "Device state: Config";
                    break;
                case 60: // Need Auth
                    qDebug() << "Device state: Need Auth";
                    break;
                case 70: // Ip Config
                    qDebug() << "Device state: Ip Config";
                    break;
                case 80: // Ip Check
                    qDebug() << "Device state: Ip Check";
                    break;
                case 90: // Secondaries
                    qDebug() << "Device state: Secondaries";
                    break;
                case 100: // Activated
                    qDebug() << "Device state: Activated";
                    break;
                case 110: // Deactivating
                    qDebug() << "Device state: Deactivating";
                    break;
                case 120: // Failed
                    qDebug() << "Device state: Failed";
                    break;
                default:
                    qDebug() << "Device state: Unknown state" << state;
                    break;
            }
        }
        if (changedProperties.contains("ActiveConnection"))
        {
            QDBusObjectPath activeConnectionPath = changedProperties["ActiveConnection"].value<QDBusObjectPath>();
            if (activeConnectionPath.path().isEmpty()) {
                qDebug() << "Device has no active connection.";
            } else {
                qDebug() << "Device active connection:" << activeConnectionPath.path();
            }
        }
    }
}

void NetworkConnection::onPropertiesChanged(const QString& interfaceName, const QVariantMap& changedProperties, const QStringList& invalidatedProperties)
{
    if (interfaceName == "org.freedesktop.NetworkManager.Connection.Active")
    {
        uint state = changedProperties["State"].toUInt();  // Get the connection state from the properties

        // Handle different states (activate, deactivating, etc.)
        switch (state)
        {
            case 1:  // Activating
                qDebug() << "Network is activating.";
                break;
            case 2:  // Active
                qDebug() << "Network is active.";
                break;
            case 3:  // Deactivating
                qDebug() << "Network is deactivating.";
                break;
            case 4:  // Deactivated
                qDebug() << "Network is deactivated.";
                break;
            default:
                break;
        }
    }
}
