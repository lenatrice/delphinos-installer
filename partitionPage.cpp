#include "partitionPage.hpp"
#include <kpmcore/backend/corebackendmanager.h>
#include <kpmcore/backend/corebackend.h>
#include <kpmcore/core/device.h>
#include <kpmcore/core/partitiontable.h>
#include <kpmcore/core/devicescanner.h>
#include <kpmcore/core/operationstack.h>
#include <QVariant>
#include <QDebug>
#include <cmath>

Q_DECLARE_METATYPE(Device*);

PartitionPage::PartitionPage(QWidget* parent) : QWidget(parent)
{
    qDebug() << "Creating partition page";

    page = new PageContent(
        "Particionamento de discos",
        "Selecione o espaço livre no dispositivo em que deseja instalar o DelphinOS",
        640, 480, this
    );

    formLayout = new QFormLayout;
    deviceCombobox = new QComboBox;
    partitionTable = new QTableWidget;
    
    QStringList headers;
    headers << "Node" << "Label" << "Type" << "Size";
    partitionTable->setColumnCount(4);
    partitionTable->setHorizontalHeaderLabels(headers);
    partitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    partitionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    formLayout->addRow("Dispositivo:", deviceCombobox);
    formLayout->setAlignment(Qt::AlignLeft);
    QWidget* dummy = new QWidget;
    formLayout->addRow("Tabela de partições:", dummy);
    formLayout->addRow(partitionTable);



    page->addLayout(formLayout);
}

void PartitionPage::initialize()
{
    if (hasInitialized) { return; }
    hasInitialized = true;

    qDebug() << "Initializing partition page";

    QByteArray env = qgetenv("KPMCORE_BACKEND");
    auto backendName = env.isEmpty() ? CoreBackendManager::defaultBackendName() : env;

    qDebug() << "Loading backend:" << backendName;

    if (!CoreBackendManager::self()->load(backendName))
    {
        qWarning() << "Failed to load KPMCore backend" << backendName;
        return;
    };

    qDebug() << "Backend loaded successfully";

    OperationStack* operationStack = new OperationStack(this);
    DeviceScanner* deviceScanner = new DeviceScanner(this, *operationStack);
    deviceScanner->scan();

    QList<Device*> devices = operationStack->previewDevices();

    for (Device* device : devices)
    {   
        deviceCombobox->addItem(device->deviceNode(), QVariant::fromValue(device));
    }

    connect(deviceCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this, &PartitionPage::onDeviceChanged);
    connect(partitionTable, QOverload<QTableWidgetItem*, QTableWidgetItem*>::of(&QTableWidget::currentItemChanged), this, &PartitionPage::onPartitionItemChanged);
    deviceCombobox->setCurrentIndex(0);
    onDeviceChanged(0);

}

void PartitionPage::onDeviceChanged(int index)
{
    const Device* device = deviceCombobox->currentData(Qt::UserRole).value<Device*>();
    const PartitionTable* partTable = device->partitionTable();

    partitionTable->setRowCount(0);

    for (const Partition* part : partTable->children()) {
        const QString partNode = part->deviceNode();
        const QString partLabel = part->label();
        const QString partType = part->fileSystem().name();
        const qint64 partSectorSize = part->sectorSize();
        const QString partSizeGb = QString::number((static_cast<double>(part->sectorSize()) * static_cast<double>(part->length()) / 1000000000), 'f', 2) + " GB";

        int row = partitionTable->rowCount();

        QTableWidgetItem* partNodeItem = new QTableWidgetItem(partNode);
        partNodeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partLabelItem = new QTableWidgetItem(partLabel);
        partLabelItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partTypeItem = new QTableWidgetItem(partType);
        partTypeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partSizeItem = new QTableWidgetItem(partSizeGb);
        partSizeItem->setData(partitionRole, QVariant::fromValue(part));

        partitionTable->insertRow(row);

        partitionTable->setItem(row, 0, partNodeItem);
        partitionTable->setItem(row, 1, partLabelItem);
        partitionTable->setItem(row, 2, partTypeItem);
        partitionTable->setItem(row, 3, partSizeItem);
    }

    partitionTable->resizeColumnsToContents();
}

void PartitionPage::onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem)
{

}