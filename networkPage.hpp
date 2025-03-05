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