#include "partitionPage.hpp"
#include <kpmcore/backend/corebackendmanager.h>
#include <kpmcore/backend/corebackend.h>
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

    page->setRequireConfirmation(true);
    page->setCanAdvance(false);

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

    for (const Device* device : devices)
    {   
        deviceCombobox->addItem(device->deviceNode() + " (" + getSize(device) + ")", QVariant::fromValue(device));
    }

    connect(deviceCombobox, &QComboBox::currentIndexChanged, this, &PartitionPage::onDeviceChanged);
    connect(partitionTable, &QTableWidget::currentItemChanged, this, &PartitionPage::onPartitionItemChanged);
    deviceCombobox->setCurrentIndex(0);
    onDeviceChanged(0);

}

void PartitionPage::onDeviceChanged(int index)
{
    const Device* device = deviceCombobox->currentData(Qt::UserRole).value<Device*>();

    if (!device)
    {
        qWarning() << "Invalid device pointer";
    }

    const PartitionTable* partTable = device->partitionTable();

    if (!partTable)
    {
        qWarning() << "Invalid partition table";
    }

    partitionTable->clearContents();
    partitionTable->setRowCount(0);

    for (const Partition* part : partTable->children()) {
        const QString partNode = part->deviceNode();
        const QString partLabel = part->label();
        const QString partType = part->fileSystem().name();
        const QString partSizeGb = getSize(part);

        int row = partitionTable->rowCount();
        partitionTable->insertRow(row);

        QTableWidgetItem* partNodeItem = new QTableWidgetItem(partNode);
        partNodeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partLabelItem = new QTableWidgetItem(partLabel);
        partLabelItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partTypeItem = new QTableWidgetItem(partType);
        partTypeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partSizeItem = new QTableWidgetItem(partSizeGb);
        partSizeItem->setData(partitionRole, QVariant::fromValue(part));


        partitionTable->setItem(row, 0, partNodeItem);
        partitionTable->setItem(row, 1, partLabelItem);
        partitionTable->setItem(row, 2, partTypeItem);
        partitionTable->setItem(row, 3, partSizeItem);
    }

    partitionTable->resizeColumnsToContents();
}

void PartitionPage::onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem)
{
    page->setConfirmationMessage("");
    page->setCanAdvance(false);

    if (!currentItem)
    {
        return;
    }

    const QVariant itemData = currentItem->data(partitionRole);

    if (!itemData.canConvert<const Partition*>())
    {
        qWarning() << "Item does not contain a valid Partition pointer";
        return;
    }

    const Partition* partition = itemData.value<const Partition*>();

    if (!partition)
    {
        qWarning() << "Selected Partition is null";
        return;
    }

    if (partition->roles().has(PartitionRole::Unallocated))
    {
        qDebug() << "Selected valid free space for installation";
    } else {
        qWarning() << "Selected partition is not free space";
        return;
    }

    if (static_cast<double>(partition->capacity()) / GiB < 10) // If selected space has less than 10GiB, issue a warning
    {
        page->setRequireWarning(true);
        page->setWarningMessage("O espaço livre selecionado de " + getSize(partition) + " pode ser insuficiente para instalar o sistema, " + 
        "ou pode gerar problemas futuros conforme mais espaço for necessário. É recomendado que você selecione um espaço livre maior do que 10 GiB.\n\nContinuar mesmo assim?");
    } else {
        page->setRequireWarning(false);
    }

    page->setConfirmationMessage("Instalar DelphinOS em espaço livre de " + getSize(partition) + "?");
    page->setCanAdvance(true);
}