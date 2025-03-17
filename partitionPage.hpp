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
#include <QTableWidget>
#include <kpmcore/core/devicescanner.h>
#include <kpmcore/core/device.h>
#include <kpmcore/core/partition.h>
#include <kpmcore/core/operationstack.h>
#include <kpmcore/ops/operation.h>
#include <kpmcore/util/report.h>
#include <QDoubleSpinBox>

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
    OperationStack* operationStack;
    DeviceScanner* deviceScanner;

    Report* rootReport;

    Partition* newBootPartition = nullptr;
    Partition* newRootPartition = nullptr;
    bool newSystemPartitionsDeleted = true;

    QDoubleSpinBox* systemSizeSpinbox;
    qint64 maxSystemSizeBytes = 0;       // Maximum system size in bytes
    double maxSystemSizeRoundedGib = 0.; // Maximum system size in GiB rounded 
    
    bool hasInitialized = false;
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        initialize();
    }

    // Page contents
    PageContent* page;

    // Device selection and control
    QComboBox* deviceCombobox;
    QPushButton* rescanDevicesButton;
    const int deviceRole = Qt::UserRole;

    // Partition table widget
    QTableWidget* partitionTableWidget;
    const int partitionRole = Qt::UserRole;

    Partition* selectedPartition = nullptr;

    // Function buttons
    QPushButton* createPartitionButton;
    QPushButton* deletePartitionButton;
    QPushButton* unmountPartitionButton;
    QPushButton* mountPartitionButton;

    QPushButton* newPartitionTableButton;
    QPushButton* createSystemPartitionsButton;

    // Update partition table
    void updatePartitionTable();

    // Run operations in operation stack
    void runOperations();

    // Scan all devices and repopulate deviceCombobox
    void scanDevices();

    qint32 countPrimaryPartitions(Device* device)
    {
        if (!device)
        {
            qWarning() << "countPrimaryPartitions(): Device is nullptr";
            return -1;
        }

        qint32 count = 0;

        for (Partition* part : device->partitionTable()->children())
        {
            if (!part)
            {
                qWarning() << "countPrimaryPartitions(): Partition is nullptr";
                continue;
            }

            if (part->roles().has(PartitionRole::Primary))
            {
                count++;
            }
        }
        return count;
    }

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
    
    // Determine path to create a new partition
    QString determineNewPartitionNodePath(const QString deviceNode, const int partitionNumber)
    {
        if      (deviceNode.startsWith("/dev/nvme"))                                       return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/sd") || deviceNode.startsWith("/dev/mmcblk")) return QString("%1%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/loop"))                                       return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/md"))                                         return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/hd"))                                         return QString("%1%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/dasd"))                                       return QString("%1%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/pmem"))                                       return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/zd"))                                         return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/mapper/mpath"))                               return QString("%1p%2").arg(deviceNode).arg(partitionNumber);
        else if (deviceNode.startsWith("/dev/drbd"))                                       return QString("%1%2").arg(deviceNode).arg(partitionNumber);
        else throw std::runtime_error("Unsupported device type");
    }

    // Get currently selected device. May return nullptr.
    Device* getSelectedDevice()
    {
        int currentIndex = deviceCombobox->currentIndex();
    
        if (currentIndex == -1) 
        {
            qWarning() << "getSelectedDevice(): No device selected";
            return nullptr;
        }

        QVariant deviceItemData = deviceCombobox->itemData(currentIndex, deviceRole);

        if (!deviceItemData.canConvert<Device*>())
        {
            qWarning() << "getSelectedDevice(): Item does not contain a valid Device pointer";
            return nullptr;
        }
    
        Device* device = deviceItemData.value<Device*>();
    
        return device;
    }

    // Get currently selected partition. May return nullptr.
    Partition* getSelectedPartition()
    {
        QTableWidgetItem* currentItem = partitionTableWidget->currentItem();
    
        if (!currentItem) 
        {
            qWarning() << "getSelectedPartition(): No partition selected";
            return nullptr;
        }

        QVariant partitionItemData = currentItem->data(partitionRole);
        
        if (!partitionItemData.canConvert<Partition*>())
        {
            qWarning() << "getSelectedPartition(): Item does not contain a valid Partition pointer";
            return nullptr;
        }
        
        Partition* partition = partitionItemData.value<Partition*>();

        return partition;
    }

private slots:
    void onDeviceChanged(int index);
    void onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem);
    void onCreatePartitionButtonClicked(bool checked);
    void onDeletePartitionButtonClicked(bool checked);
    void onMountPartitionButtonClicked(bool checked);
    void onUnmountPartitionButtonClicked(bool checked);
    void onNewPartitionTableButtonClicked(bool checked);
    void onCreateSystemPartitionsButtonClicked(bool checked);
    
public: 
    PageContent* getPage()
    {
        return page;
    };
    
    PartitionPage(QWidget* parent);

    void initialize();
};
