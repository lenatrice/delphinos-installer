#include "mainWindow.hpp"
#include <QTableWidget>
#include <kpmcore/core/device.h>
#include <kpmcore/core/partition.h>

// Constant values of storage size in bytes
const qint64 KiB = 1024;
const qint64 MiB = KiB * 1024;
const qint64 GiB = MiB * 1024;
const qint64 TiB = GiB * 1024;
const qint64 PiB = TiB * 1024;

class PartitionPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;
    QComboBox* deviceCombobox;
    QTableWidget* partitionTable;
    const int partitionRole = Qt::UserRole;
    bool hasInitialized = false;
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        initialize();
    }
    Device* selectedDevice;
    Partition* selectedPartition;

    // Function buttons
    QPushButton* deletePartition;

    // Get human-readable size of device
    QString getSize(const Device* device)
    {
        qint64 size = device->capacity();
    
        if      (size < KiB) return QString::number(size)                                    + " Bytes";
        else if (size < MiB) return QString::number(static_cast<double>(size) / KiB, 'f', 2) + " KiB";
        else if (size < GiB) return QString::number(static_cast<double>(size) / MiB, 'f', 2) + " MiB";
        else if (size < TiB) return QString::number(static_cast<double>(size) / GiB, 'f', 2) + " GiB";
        else if (size < PiB) return QString::number(static_cast<double>(size) / TiB, 'f', 2) + " TiB";
        else                 return QString::number(static_cast<double>(size) / PiB, 'f', 2) + " PiB";
    }

    // Get human-readable size of partition
    QString getSize(const Partition* partition)
    {
        qint64 size = partition->capacity();
    
        if      (size < KiB) return QString::number(size)                                    + " Bytes";
        else if (size < MiB) return QString::number(static_cast<double>(size) / KiB, 'f', 2) + " KiB";
        else if (size < GiB) return QString::number(static_cast<double>(size) / MiB, 'f', 2) + " MiB";
        else if (size < TiB) return QString::number(static_cast<double>(size) / GiB, 'f', 2) + " GiB";
        else if (size < PiB) return QString::number(static_cast<double>(size) / TiB, 'f', 2) + " TiB";
        else                 return QString::number(static_cast<double>(size) / PiB, 'f', 2) + " PiB";
    }
    

private slots:
    void onDeviceChanged(int index);
    void onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem);
    
public: 
    PageContent* getPage()
    {
        return page;
    };
    
    PartitionPage(QWidget* parent);

    void initialize();


};
