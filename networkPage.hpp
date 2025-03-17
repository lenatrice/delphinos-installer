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

#include "mainWindow.hpp"
#include <QMap>


class NetworkPage : QWidget
{
Q_OBJECT
private:
    // Network
    PageContent* page;

    QFormLayout* formLayout;
    QListWidget* deviceList;

    // Wi-Fi access points list
    QListWidget* wifiAccessPointList;

    // Network function buttons
    QHBoxLayout* functionButtons;
    QPushButton* refreshButton;
    QPushButton* disconnectButton;
    QPushButton* connectButton;

    void populateNetworkDevices();
    void updateNetworkSettings(QDBusObjectPath connectionPath, QMap<QString, QMap<QString, QVariant>> nmSettings);
    QString requestAccessPointPassword(const QString& wifiAccessPoint);
    const int networkDeviceObjRole = Qt::UserRole;
    const int wifiAccessPointObjRole = Qt::UserRole;
    const int networkConnectionObjRole = Qt::UserRole;

    QList<NetworkDevice*> networkDevices;
    QList<NetworkConnection*> networkConnections;
    
    NetworkDevice* getNetworkDevice(QListWidgetItem* networkDeviceItem)
    {
        if (networkDeviceItem!=nullptr)
        {
            return networkDeviceItem->data(networkDeviceObjRole).value<NetworkDevice*>();
        } else {
            return nullptr;
        }
    }
    
    WifiAccessPoint* getAccessPoint(QListWidgetItem* accessPointItem)
    {    
        if (accessPointItem!=nullptr)
        {
            return accessPointItem->data(networkDeviceObjRole).value<WifiAccessPoint*>();
        } else {
            return nullptr;
        }
    }
    
    NetworkConnection* getNetworkConnection(QListWidgetItem* networkConnectionItem)
    {
        if (networkConnectionItem!=nullptr)
        {
            return networkConnectionItem->data(networkConnectionObjRole).value<NetworkConnection*>();
        } else {
            return nullptr;
        }
    }

    void clearNetworkDevices(QList<NetworkDevice*>& list, QListWidget* networkDeviceList)
    {
        qDeleteAll(list);
        list.clear();

        networkDeviceList->clear();
    }

private slots:
    void updateNetworkDevice();
    void updateConnectionSettings(QDBusObjectPath connectionPath, ConnectionSettings nmSettings);
    void connectNetwork();

public:
    NetworkPage(QWidget* parent);
    PageContent* getPage()
    {
        return page;
    }; 
};
