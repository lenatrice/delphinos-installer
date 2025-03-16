#include "partitionPage.hpp"
#include <kpmcore/backend/corebackendmanager.h>
#include <kpmcore/backend/corebackend.h>
#include <kpmcore/core/partitiontable.h>
#include <kpmcore/ops/newoperation.h>
#include <kpmcore/ops/deleteoperation.h>
#include <kpmcore/ops/createpartitiontableoperation.h>
#include <kpmcore/ops/createfilesystemoperation.h>
#include <kpmcore/fs/fat32.h>
#include <kpmcore/fs/ext4.h>
#include <kpmcore/fs/filesystemfactory.h>
#include <QVariant>
#include <QDebug>
#include <QProcess>
#include <QLineEdit>
#include <cmath>

Q_DECLARE_METATYPE(Device*);

PartitionPage::PartitionPage(QWidget* parent) : QWidget(parent)
{
    qDebug() << "Creating partition page";

    page = new PageContent(
        "Particionamento de discos",
        "Selecione o espaço livre no dispositivo em que deseja instalar o sistema, e clique em \"Criar partições do sistema\".",
        740, 480, this
    );

    QVBoxLayout* layout = new QVBoxLayout;
    QFormLayout* deviceFormLayout = new QFormLayout;
    QFormLayout* spinboxFormLayout = new QFormLayout;
    layout->addLayout(deviceFormLayout);
    layout->addLayout(spinboxFormLayout);
    
    // Device widget
    QHBoxLayout* deviceLayout = new QHBoxLayout;
    deviceCombobox = new QComboBox;
    rescanDevicesButton = new QPushButton("Reescanear dispositivos");

    connect(rescanDevicesButton, &QPushButton::clicked, this, [this](bool checked){
        scanDevices();
    });
    
    deviceLayout->addWidget(deviceCombobox);
    deviceLayout->addWidget(rescanDevicesButton);
    deviceFormLayout->addRow("Dispositivo:", deviceLayout);
    
    // Partition table widget
    QHBoxLayout* partitionLayout = new QHBoxLayout;
    partitionTableWidget = new QTableWidget;
    QStringList headers;
    headers << "Node" << "Label" << "Type" << "Size" << "Mountpoint";
    partitionTableWidget->setColumnCount(5);
    partitionTableWidget->setHorizontalHeaderLabels(headers);
    partitionTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    partitionTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    partitionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    partitionLayout->addWidget(partitionTableWidget);

    deviceFormLayout->addRow(new QLabel("Tabela de partições:"));

    // Function buttons
    QVBoxLayout* partitionFunctionButtons = new QVBoxLayout;
    createPartitionButton = new QPushButton("Criar partição");
    deletePartitionButton = new QPushButton("Deletar partição");
    mountPartitionButton = new QPushButton("Montar partição");
    unmountPartitionButton = new QPushButton("Desmontar partição");
    partitionFunctionButtons->addWidget(createPartitionButton);
    partitionFunctionButtons->addWidget(deletePartitionButton);
    partitionFunctionButtons->addWidget(mountPartitionButton);
    partitionFunctionButtons->addWidget(unmountPartitionButton);
    partitionLayout->addLayout(partitionFunctionButtons);

    QHBoxLayout* functionButtons = new QHBoxLayout;

    newPartitionTableButton = new QPushButton("Nova tabela de partições");
    createSystemPartitionsButton = new QPushButton("Criar partições do sistema");


    functionButtons->addWidget(newPartitionTableButton);
    functionButtons->addWidget(createSystemPartitionsButton);

    deviceFormLayout->addRow(partitionLayout);
    deviceFormLayout->addRow(functionButtons);
    createPartitionButton->setEnabled(false);
    deletePartitionButton->setEnabled(false);
    mountPartitionButton->setEnabled(false);
    unmountPartitionButton->setEnabled(false);
    createSystemPartitionsButton->setEnabled(false);

    connect(newPartitionTableButton, &QPushButton::clicked, this, [this](bool checked)
    {
        newPartitionTableButton->setEnabled(false);
        deletePartitionButton->setEnabled(false);
        createSystemPartitionsButton->setEnabled(false);
        PartitionPage::onNewPartitionTableButtonClicked(checked);
        newPartitionTableButton->setEnabled(true);
        deletePartitionButton->setEnabled(true);
    });

    connect(createPartitionButton, &QPushButton::clicked, this, [this](bool checked)
    {
        createPartitionButton->setEnabled(false);
        PartitionPage::onCreatePartitionButtonClicked(checked);
        createPartitionButton->setEnabled(true);
    });

    connect(deletePartitionButton, &QPushButton::clicked, this, [this](bool checked)
    {
        deletePartitionButton->setEnabled(false);
        PartitionPage::onDeletePartitionButtonClicked(checked);
        deletePartitionButton->setEnabled(true);
    });

    connect(mountPartitionButton, &QPushButton::clicked, this, [this](bool checked)
    {
        mountPartitionButton->setEnabled(false);
        deletePartitionButton->setEnabled(false);
        PartitionPage::onMountPartitionButtonClicked(checked);
        mountPartitionButton->setEnabled(true);
        deletePartitionButton->setEnabled(true);
    });
    connect(unmountPartitionButton, &QPushButton::clicked, this, [this](bool checked)
    {
        unmountPartitionButton->setEnabled(false);
        deletePartitionButton->setEnabled(false);
        PartitionPage::onUnmountPartitionButtonClicked(checked);
        unmountPartitionButton->setEnabled(true);
        deletePartitionButton->setEnabled(true);
    });

    connect(createSystemPartitionsButton, &QPushButton::clicked, this, [this](bool checked)
    {
        createSystemPartitionsButton->setEnabled(false);
        newPartitionTableButton->setEnabled(false);
        PartitionPage::onCreateSystemPartitionsButtonClicked(checked);
        createSystemPartitionsButton->setEnabled(true);
        newPartitionTableButton->setEnabled(true);
    });

    // System size spinbox
    systemSizeSpinbox = new QDoubleSpinBox;
    systemSizeSpinbox->setEnabled(false);
    systemSizeSpinbox->setDecimals(2);
    systemSizeSpinbox->setSuffix("GiB");
    systemSizeSpinbox->setRange(0., 0.);
    systemSizeSpinbox->setValue(0.);
    spinboxFormLayout->addRow("Espaço utilizado pelo sistema:", systemSizeSpinbox);

    page->addLayout(layout);
}

void PartitionPage::initialize()
{
    if (hasInitialized) { return; }
    hasInitialized = true;

    page->setRequireConfirmation(true);
    page->setCanAdvance(false);

    rootReport = new Report(nullptr, "Partitioning operation");

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

    operationStack = new OperationStack(this);
    deviceScanner = new DeviceScanner(this, *operationStack);
    scanDevices();

    connect(deviceCombobox, &QComboBox::currentIndexChanged, this, &PartitionPage::onDeviceChanged);
    connect(partitionTableWidget, &QTableWidget::currentItemChanged, this, &PartitionPage::onPartitionItemChanged);
    deviceCombobox->setCurrentIndex(0);
    onDeviceChanged(0);
}

void PartitionPage::scanDevices()
{
    if (!deviceScanner)
    {
        qWarning() << "Device scanner is nullptr";
        return;
    }

    rescanDevicesButton->setEnabled(false);

    // Store the currently selected device node 
    QString selectedDeviceNode;
    if (deviceCombobox->currentIndex() >= 0) {
        Device* selectedDevice = deviceCombobox->currentData().value<Device*>();
        selectedDeviceNode = selectedDevice->deviceNode();
    }

    // Rescan devices
    deviceScanner->scan();

    // Get the updated list of devices
    QList<Device*> devices = operationStack->previewDevices();

    // Clear the combobox
    deviceCombobox->clear();

    // Repopulate the combobox
    for (Device* device : devices) {
        deviceCombobox->addItem(device->deviceNode() + " (" + getSize(device) + ")", QVariant::fromValue(device));
    }

    // Restore the previously selected device
    if (!selectedDeviceNode.isEmpty()) {
        for (int i = 0; i < deviceCombobox->count(); ++i) {
            Device* device = deviceCombobox->itemData(i).value<Device*>();
            if (device->deviceNode() == selectedDeviceNode) {
                deviceCombobox->setCurrentIndex(i);
                break;
            }
        }
    }

    rescanDevicesButton->setEnabled(true);
}

void PartitionPage::runOperations()
{
    qDebug() << "Number of operations in stack:" << operationStack->size();
    for (Operation* op : operationStack->operations()) {
        if (!op) {
            qWarning() << "Found nullptr in operationStack";
            continue;
        }
        qDebug() << "Operation:" << op->description();

        Report* childReport = new Report(rootReport, op->description());

        op->execute(*childReport);

        qDebug() << childReport->status();

        delete childReport;
    }
    qDebug() << "Finished all operations. Clearing operation stack";
    operationStack->clearOperations();
    qDebug() << "Operation stack is clear";
}

void PartitionPage::updatePartitionTable()
{
    Device* device = deviceCombobox->currentData(Qt::UserRole).value<Device*>();

    if (!device)
    {
        qWarning() << "updatePartitionTable(): Invalid device pointer";
        return;
    }

    PartitionTable* partTable = device->partitionTable();

    if (!partTable)
    {
        qWarning() << "updatePartitionTable(): Invalid partition table";
        return;
    }

    partitionTableWidget->clearContents();
    partitionTableWidget->setRowCount(0);

    qDebug() << "Found partition table of type:" << partTable->typeName();
    qDebug() << "Partition table size:" << partTable->children().count();

    for (Partition* part : partTable->children()) {
        const QString partNode = part->deviceNode();
        const QString partLabel = part->label();
        const QString partType = part->fileSystem().name();
        const QString partSizeGb = getSize(part);
        QString partMountpoint;

        if (part->isMounted())
        {
            partMountpoint = FileSystem::detectMountPoint(&part->fileSystem(), part->deviceNode());
        } else {
            partMountpoint = "Não montado";
        }

        qDebug() << "Found partition " + partNode + " of type " + partType;

        int row = partitionTableWidget->rowCount();
        partitionTableWidget->insertRow(row);

        QTableWidgetItem* partNodeItem = new QTableWidgetItem(partNode);
        partNodeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partLabelItem = new QTableWidgetItem(partLabel);
        partLabelItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partTypeItem = new QTableWidgetItem(partType);
        partTypeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partSizeItem = new QTableWidgetItem(partSizeGb);
        partSizeItem->setData(partitionRole, QVariant::fromValue(part));
        QTableWidgetItem* partMountpointItem = new QTableWidgetItem(partMountpoint);
        partMountpointItem->setData(partitionRole, QVariant::fromValue(part));

        partitionTableWidget->setItem(row, 0, partNodeItem);
        partitionTableWidget->setItem(row, 1, partLabelItem);
        partitionTableWidget->setItem(row, 2, partTypeItem);
        partitionTableWidget->setItem(row, 3, partSizeItem);
        partitionTableWidget->setItem(row, 4, partMountpointItem);
    }

    partitionTableWidget->resizeColumnsToContents();
}

void PartitionPage::onDeviceChanged(int index)
{
    systemSizeSpinbox->setEnabled(false);
    systemSizeSpinbox->setRange(0., 0.);
    systemSizeSpinbox->setValue(0.);

    Device* device = getSelectedDevice();

    if (!device)
    {
        qWarning() << "onDeviceChanged(): Invalid device pointer";
        return;
    }


    updatePartitionTable();
}


void PartitionPage::onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem)
{
    page->setConfirmationMessage("");
    createSystemPartitionsButton->setEnabled(false);

    if (!currentItem) return;

    // Get currently selected Partition and determine if it is a valid unallocated space for installation.
    
    Partition* partition = getSelectedPartition();

    if (!partition)
    {
        qWarning() << "onPartitionItemChanged(): Selected Partition is null";
        return;
    }

    if (partition->roles().has(PartitionRole::Unallocated))
    {
        createPartitionButton->setEnabled(true);
        deletePartitionButton->setEnabled(false);
        mountPartitionButton->setEnabled(false);
        unmountPartitionButton->setEnabled(false);

        maxSystemSizeBytes = partition->capacity(); // Full precision maximum size
        maxSystemSizeRoundedGib = std::round((static_cast<double>(maxSystemSizeBytes) / static_cast<double>(GiB)) * 100.0) / 100.0; // Rounded maximum size
        systemSizeSpinbox->setEnabled(true);
        systemSizeSpinbox->setRange(10., maxSystemSizeRoundedGib);
        systemSizeSpinbox->setValue(maxSystemSizeRoundedGib);
    } else {
        createPartitionButton->setEnabled(false);
        deletePartitionButton->setEnabled(true);
        mountPartitionButton->setEnabled(true);
        unmountPartitionButton->setEnabled(true);

        maxSystemSizeBytes = 0;
        maxSystemSizeRoundedGib = 0.;
        systemSizeSpinbox->setEnabled(false);
        systemSizeSpinbox->setRange(0., 0.);
        systemSizeSpinbox->setValue(0.);
        return;
    }

    // Update confirmation message and allow creation of system partitions.
    createSystemPartitionsButton->setEnabled(true);
}

void PartitionPage::onCreatePartitionButtonClicked(bool checked)
{
    Partition* partition = getSelectedPartition();

    if (!partition)
    {
        qWarning() << "onCreatePartitionButtonClicked(): Selected partition is null";
        return;
    }

    Device* device = getSelectedDevice();

    if (!device)
    {
        qWarning() << "onCreatePartitionButtonClicked(): Selected device is null";
        return;
    }

    QPointer<QDialog> createPartitionDialog = new QDialog;
    createPartitionDialog->setWindowTitle("Selecione o tipo da partição");
    QVBoxLayout* dialogLayout = new QVBoxLayout;
    QFormLayout* dialogForm = new QFormLayout;

    QComboBox* partitionTypeCombobox = new QComboBox;
    partitionTypeCombobox->addItem("Ext4");
    partitionTypeCombobox->addItem("Fat32");
    partitionTypeCombobox->addItem("NTFS");
    partitionTypeCombobox->setCurrentText("ext4");
    dialogForm->addRow("Sistema de arquivos:", partitionTypeCombobox);

    QDoubleSpinBox* partitionSizeSpinbox = new QDoubleSpinBox;
    qint64 maxPartitionSizeBytes = partition->capacity(); // Full precision maximum size
    double maxPartitionSizeRoundedGib = std::round((static_cast<double>(maxPartitionSizeBytes) / static_cast<double>(GiB)) * 100.0) / 100.0; // Rounded maximum size
    partitionSizeSpinbox->setEnabled(true);
    partitionSizeSpinbox->setRange(0.1, maxPartitionSizeBytes);
    partitionSizeSpinbox->setValue(maxPartitionSizeRoundedGib);
    dialogForm->addRow("Tamanho da partição:", partitionSizeSpinbox);

    QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogLayout->addLayout(dialogForm);
    dialogLayout->addWidget(dialogButtons);

    createPartitionDialog->setLayout(dialogLayout);

    connect(dialogButtons, &QDialogButtonBox::accepted, createPartitionDialog, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, createPartitionDialog, &QDialog::reject);

    FileSystem* newFileSystem;
    qint64 partitionSize;

    if (createPartitionDialog->exec() == QDialog::Accepted)
    {

        if (partitionSizeSpinbox->value() == maxPartitionSizeRoundedGib) partitionSize = maxPartitionSizeBytes;
        else partitionSize = partitionSizeSpinbox->value() * GiB;

        if (partitionTypeCombobox->currentText() == "Ext4")
        {
            newFileSystem = FileSystemFactory::create(FileSystem::Type::Ext4,
                partition->firstSector(), partitionSize / partition->sectorSize(), partition->sectorSize()
            );
        }
        else if (partitionTypeCombobox->currentText() == "Fat32")
        {
            newFileSystem = FileSystemFactory::create(FileSystem::Type::Fat32,
                partition->firstSector(), partitionSize / partition->sectorSize(), partition->sectorSize()
            );
        }
        else if (partitionTypeCombobox->currentText() == "NTFS")
        {
            newFileSystem = FileSystemFactory::create(FileSystem::Type::Ntfs,
                partition->firstSector(), partitionSize / partition->sectorSize(), partition->sectorSize()
            );
        }
    }

    QString partitionNode = determineNewPartitionNodePath(device->deviceNode(), countPrimaryPartitions(device));

    PartitionTable* devicePartitionTable = device->partitionTable();

    PartitionTable::Flags availableFlags;
    for (PartitionTable::Flag _flag : devicePartitionTable->flagList())
    {
        availableFlags |= _flag;
    }
    
    Partition* newPartition = new Partition(
        devicePartitionTable,                     // PartitionTable
        *device,                                  // Device
        PartitionRole(PartitionRole::Primary),    // Partition Role
        newFileSystem,                            // Filesystem 
        partition->firstSector(),                 // First Sector
        partitionSize / partition->sectorSize(),  // Last sector
        partitionNode,                            // Partition node path
        availableFlags,                           // Available flags
        "",                                       // Mount point
        false,                                    // Not mounted
        PartitionTable::Flag::None                // No flag
    );

    NewOperation* createPartition = new NewOperation(*device, newPartition);

    if (!createPartition->canCreateNew(partition))
    {
        qWarning() << "Cannot create new partition";
        return;
    }

    qDebug() << "Pushing the operations to create partitions to the operation stack";
    operationStack->push(createPartition);

    runOperations();

    newFileSystem->create(*rootReport, partitionNode);

    updatePartitionTable();
}

void PartitionPage::onDeletePartitionButtonClicked(bool checked)
{
    Partition* partition = getSelectedPartition();
    
    if (!partition)
    {
        qWarning() << "onDeletePartitionButtonClicked(): Invalid partition pointer";
        return;
    }

    Device* device = getSelectedDevice();

    if (!device)
    {
        qWarning() << "onDeletePartitionButtonClicked(): Invalid device pointer";
        return;
    }

    partitionTableWidget->clearContents();
    partitionTableWidget->setRowCount(0);

    // If the selected partition to delete is from DelphinOS, delete both partitions
    if (newBootPartition && newRootPartition)
    {
        qDebug() << "newBootPartition and newRootPartition found";
        if (partition->deviceNode() == newBootPartition->deviceNode() || partition->deviceNode() == newRootPartition->deviceNode())
        {
            // Wait for user confirmation before deleting partition
            QMessageBox::StandardButtons warning;
            warning = QMessageBox::warning(this, "Atenção", "Essa operação irá deletar as partições do DelphinOS "
                + newBootPartition->deviceNode() + " e " + newRootPartition->deviceNode() + ".\n\nDeseja continuar?", QMessageBox::Ok | QMessageBox::Cancel); 

            if (warning == QMessageBox::Cancel) return;

            DeleteOperation* deleteNewBootPartitionOperation = new DeleteOperation(*device, newBootPartition);
            DeleteOperation* deleteNewRootPartitionOperation = new DeleteOperation(*device, newRootPartition);

            // Unmount partitions for deletion
            if (newRootPartition->isMounted())
            {
                if (!newRootPartition->unmount(*rootReport))
                {
                    qWarning() << "Could not unmount newRootPartition";
                    updatePartitionTable();
                    return;
                }
            }

            if (newBootPartition->isMounted())
            {
                if (!newBootPartition->unmount(*rootReport))
                {
                    qWarning() << "Could not unmount newBootPartition";
                    updatePartitionTable();
                    return;
                }
            }

            operationStack->push(deleteNewBootPartitionOperation);
            operationStack->push(deleteNewRootPartitionOperation);

            runOperations();
            updatePartitionTable();

            newSystemPartitionsDeleted = true;
            return;
        }
    }

    qDebug() << "Issuing warning";

    // If any other partition is selected, delete only that partition
    // Wait for user confirmation before deleting partition
    QMessageBox::StandardButtons warning;
    warning = QMessageBox::warning(this, "Atenção", "Essa operação irá deletar a partição " + partition->deviceNode() + " de " + getSize(partition) + ".\n\nDeseja continuar?", QMessageBox::Ok | QMessageBox::Cancel); 

    if (warning == QMessageBox::Cancel) return;

    qDebug() << "Unmounting partition" << partition->deviceNode();
    // Unmount partitions for deletion
    if (partition->isMounted())
    {
        if (!partition->unmount(*rootReport))
        {
            qWarning() << "Could not unmount" << partition->deviceNode();
            updatePartitionTable();
            return;
        }
    }

    qDebug() << "Creating delete operation";
    DeleteOperation* deleteOperation = new DeleteOperation(*device, partition);

    qDebug() << "Pushing delete operation to operation stack";
    operationStack->push(deleteOperation);

    qDebug() << "Running delete operation";
    runOperations();
    updatePartitionTable();
}

void PartitionPage::onMountPartitionButtonClicked(bool checked)
{
    Partition* partition = getSelectedPartition();

    if (!partition)
    {
        qWarning() << "onMountPartitionButtonClicked(): Invalid partition pointer";
        return;
    }

    if (partition->isMounted())
    {
        qWarning() << "Partition" << partition->deviceNode() << "is already mounted";

        QMessageBox::StandardButtons warning;
        warning = QMessageBox::warning(this, "Atenção", "Partição " + partition->deviceNode() + " já está montada. Ignorando solicitação para montar.", QMessageBox::Ok);

        return;
    }

    QString mountpoint = partition->mountPoint();
    qDebug() << "Default mount point for partition:" << mountpoint;

    QPointer<QDialog> mountpointDialog = new QDialog(this);
    mountpointDialog->setWindowTitle("Escolher ponto de montagem");
    QVBoxLayout* dialogLayout = new QVBoxLayout(mountpointDialog);
    QFormLayout* dialogForm = new QFormLayout;

    QLineEdit* mountpointLineEdit = new QLineEdit;
    mountpointLineEdit->setText(mountpoint);
    dialogForm->addRow("Ponto de montagem:", mountpointLineEdit);

    QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogLayout->addLayout(dialogForm);
    dialogLayout->addWidget(dialogButtons);

    mountpointDialog->setLayout(dialogLayout);

    connect(dialogButtons, &QDialogButtonBox::accepted, mountpointDialog, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, mountpointDialog, &QDialog::reject);

    if (mountpointDialog->exec() == QDialog::Accepted)
    {
        mountpoint = mountpointLineEdit->text();
        partition->setMountPoint(mountpoint);
    } else {
        return;
    }

    if (!partition->mount(*rootReport))
    {
        qWarning() << "It was not possible to mount partition" << partition->deviceNode() << "on" << mountpoint;
        QMessageBox::warning(this, "Atenção", "Não foi possível montar a partição " + partition->deviceNode() + " em " + mountpoint, QMessageBox::Ok);
    } else updatePartitionTable();
}

void PartitionPage::onUnmountPartitionButtonClicked(bool checked)
{
    Partition* partition = getSelectedPartition();

    if (!partition)
    {
        qWarning() << "onUnmountPartitionButtonClicked(): Invalid partition pointer";
        return;
    }

    if (newBootPartition && newRootPartition)
    {
        if (partition->deviceNode() == newBootPartition->deviceNode() || partition->deviceNode() == newRootPartition->deviceNode())
        {
            // Wait for user confirmation before deleting partition
            QMessageBox::critical(this, "Erro", "Não é possível desmontar uma partição do sistema DelphinOS pois isso irá impedir a instalação.", QMessageBox::Ok); 
            return;
        }
    }

    if (partition->isMounted())
    {
        if (!partition->unmount(*rootReport))
        {
            qWarning() << "Could not unmount" << partition->deviceNode();
            QMessageBox::warning(this, "Atenção", "Não foi possível desmontar a partição " + partition->deviceNode() + " em " + partition->mountPoint()
            + ".\n\nVerifique o ponto de montagem não está dentro de outra partição montada, e então desmonte ela antes.", QMessageBox::Ok);
            return;
        } else updatePartitionTable();
    } else {
        qWarning() << "Partition" << partition->deviceNode() << "is not mounted.";
        QMessageBox::warning(this, "Atenção", "Partição " + partition->deviceNode() + " não está montada. Ignorando solicitação para desmontar.", QMessageBox::Ok);
    }
}

void PartitionPage::onNewPartitionTableButtonClicked(bool checked)
{
    Device* device = getSelectedDevice();

    if (!device)
    {
        qWarning() << "onNewPartitionTableButtonClicked(): Invalid device pointer";
        return;
    }

    QMessageBox::StandardButtons warning;
    warning = QMessageBox::warning(this, "Atenção", "Essa operação irá apagar todos os dados do dispositivo " + device->deviceNode() + " de " + getSize(device) + ".\n\nDeseja continuar?", QMessageBox::Ok | QMessageBox::Cancel); 

    if (warning == QMessageBox::Cancel) return;

    // Open a dialog to select the new table type
    QPointer<QDialog> tableTypeDialog = new QDialog;
    tableTypeDialog->setWindowTitle("Selecione o tipo de tabela de partições");
    QVBoxLayout* dialogLayout = new QVBoxLayout;
    QFormLayout* dialogForm = new QFormLayout;

    QComboBox* tableTypeComboBox = new QComboBox;
    tableTypeComboBox->addItem("MBR");
    tableTypeComboBox->addItem("GPT");
    tableTypeComboBox->setCurrentText("GPT");
    dialogForm->addRow("Tipo de tabela de partições:", tableTypeComboBox);

    QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogLayout->addLayout(dialogForm);
    dialogLayout->addWidget(dialogButtons);

    tableTypeDialog->setLayout(dialogLayout);

    connect(dialogButtons, &QDialogButtonBox::accepted, tableTypeDialog, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, tableTypeDialog, &QDialog::reject);

    PartitionTable* newPartitionTable;
    
    CreatePartitionTableOperation* createPartitionTableOperation;

    if (tableTypeDialog->exec() == QDialog::Accepted)
    {
        QString selectedTableType = tableTypeComboBox->currentText();

        if (selectedTableType == "MBR")
        {
            // For MBR, first usable sector is 1, last usable sector is device->totalLogical() - 1
            createPartitionTableOperation = new CreatePartitionTableOperation(*device, PartitionTable::TableType::msdos);
        } else if (selectedTableType == "GPT") {
            createPartitionTableOperation = new CreatePartitionTableOperation(*device, PartitionTable::TableType::gpt);
        }
    } else return;

    if (!newPartitionTable)
    {
        qWarning() << "New partition table is empty";
        return;
    }

    operationStack->push(createPartitionTableOperation);
    runOperations();
    scanDevices();
    updatePartitionTable();
}

void PartitionPage::onCreateSystemPartitionsButtonClicked(bool checked)
{
    if (newSystemPartitionsDeleted)
    {
        qDebug() << "Resetting newBootPartition and newRootPartition pointers";
        newBootPartition = nullptr;
        newRootPartition = nullptr;
        qDebug() << "Successfully reset newBootPartition and newRootPartition pointers";
        newSystemPartitionsDeleted = false;
    } else if (newBootPartition || newRootPartition)
    {
        qWarning() << "System partitions were already created. Ignoring onCreateSystemPartitionsButtonClicked";
        QMessageBox::StandardButton warningDialog;
        warningDialog = QMessageBox::warning(this, "Atenção", "As partições do DelphinOS já foram criadas. Delete as partições se desejar criar o sistema em outro local", QMessageBox::Ok);
        return;
    }

    // Get currently selected Partition pointer and determine it is valid unallocated space for installation.
    Partition* partition = getSelectedPartition();

    if (!partition)
    {
        return;
    }

    if (partition->roles().has(PartitionRole::Unallocated))
    {
        qDebug() << "Selected valid free space for installation";
    } else {
        qWarning() << "Selected partition is not free space";
        return;
    }

    // Determine chosen system size
    qint64 systemSize = 0;
    if (systemSizeSpinbox->value() == maxSystemSizeRoundedGib)
    {
        systemSize = maxSystemSizeBytes;
    } else {
        systemSize = systemSizeSpinbox->value() * GiB;
    }

    // Get currently selected Device pointer.

    Device* device = getSelectedDevice();

    if (!device)
    {
        qWarning() << "Selected Device is null";
        return;
    }

    // If selected space has less than 10GiB, issue a warning.

    if (static_cast<double>(partition->capacity()) / GiB < 10)
    {
        QMessageBox::StandardButton warningDialog;
        warningDialog = QMessageBox::warning(this, "Atenção", "O espaço livre selecionado de " + getSize(partition) + " pode ser insuficiente para instalar o sistema, " + 
        "ou pode gerar problemas futuros conforme mais espaço for necessário. É recomendado que você selecione um espaço livre maior do que 10 GiB.\n\nContinuar mesmo assim?", QMessageBox::Ok | QMessageBox::Cancel);

        if (warningDialog == QMessageBox::Cancel)
        {
            return;
        }
    }

    // Get partition table of device
    PartitionTable* devicePartitionTable = device->partitionTable();

    // MBR partition table support only 4 primary partitions. Issue an error if it contains more than 2 partitions, as it will not be possible to create the necessary partitions for the system to run.
    if (devicePartitionTable->typeName() == "msdos" && countPrimaryPartitions(device) > 2)
    {
        QMessageBox::StandardButton errorDialog;
        errorDialog = QMessageBox::critical(this, "Erro", "Tabelas de partição do tipo MBR suportam apenas 4 partições primárias, " + 
            QString("e não será possível criar as 2 partições necessárias para instalar o sistema pois você já possui ") + QString::number(countPrimaryPartitions(device)) + " partições nesse dispositivo." + 
            "Considere deletar partições existentes ou selecionar outro dispositivo.", QMessageBox::Ok
        );
        return;
    }
    
    // Create new partitions for the system

    qint64 firstBootSector = partition->firstSector();
    qint64 lastBootSector = partition->firstSector() + (MiB * 256) / partition->sectorSize() - 1;
    qint64 firstRootSector = lastBootSector + 1;
    qint64 lastRootSector = systemSize / partition->sectorSize();

    QString bootDeviceNode = determineNewPartitionNodePath(device->deviceNode(), countPrimaryPartitions(device) + 1);
    QString rootDeviceNode = determineNewPartitionNodePath(device->deviceNode(), countPrimaryPartitions(device) + 2);

    FileSystem* newBootFs = FileSystemFactory::create(FileSystem::Type::Fat32,
        firstBootSector, lastBootSector, partition->sectorSize()
    );
        
    FileSystem* newRootFs = FileSystemFactory::create(FileSystem::Type::Ext4,
        firstRootSector, lastRootSector, partition->sectorSize()
    );
        
    PartitionTable::Flags availableFlags;
    for (PartitionTable::Flag _flag : devicePartitionTable->flagList())
    {
        availableFlags |= _flag;
    }
    
    newBootPartition = new Partition(
        devicePartitionTable,                     // PartitionTable
        *device,                                  // Device
        PartitionRole(PartitionRole::Primary),    // Partition Role
        newBootFs,                                // Filesystem 
        firstBootSector, lastBootSector,          // Size of 256 MiB
        bootDeviceNode, // Partition node path
        availableFlags,                           // Available flags
        "/mnt/new_root/boot",                     // Mount point
        false,                                    // Not mounted
        PartitionTable::Flag::Boot                // Bootable flag
    );

    newBootPartition->setLabel("DELPHINOS BOOT PARTITION");
    newBootPartition->setMountPoint("/mnt/new_root/boot");
    
    newRootPartition = new Partition(
        devicePartitionTable,                     // PartitionTable
        *device,                                  // Device
        PartitionRole(PartitionRole::Primary),    // Partition Role
        newRootFs,                                // Filesystem 
        firstRootSector, lastRootSector,          // Remaining size of the free space [TODO: add option to use less space]
        rootDeviceNode,                           // Partition node path
        availableFlags,                           // Available flags
        "/mnt/new_root/",                         // Mount point
        false,                                    // Not mounted
        PartitionTable::Flag::None                // No flag
    );
    
    newRootPartition->setLabel("DelphinOS Root Partition");
    newRootPartition->setMountPoint("/mnt/new_root");

    NewOperation* createBootPartition = new NewOperation(*device, newBootPartition);
    NewOperation* createRootPartition = new NewOperation(*device, newRootPartition);

    if (!createBootPartition->canCreateNew(partition))
    {
        qWarning() << "Cannot create new partition";
        return;
    }

    if (newBootPartition->capacity() + newRootPartition->capacity() > partition->capacity())
    {
        QMessageBox::StandardButton errorDialog;
        errorDialog = QMessageBox::critical(this, "Erro", "O espaço selecionado tem tamanho insuficiente para criar as partições do sistema.\n"
            + QString("(Tamanho ocupado pelas novas partições:") + QString::number(static_cast<double>(newBootPartition->capacity() + newRootPartition->capacity()) / GiB) + ")", QMessageBox::Ok
        );
        return;
    }

    qDebug() << "Pushing the operations to create partitions to the operation stack";
    operationStack->push(createBootPartition);
    operationStack->push(createRootPartition);

    runOperations();

    newBootFs->create(*rootReport, bootDeviceNode);
    newRootFs->create(*rootReport, rootDeviceNode);

    newBootPartition->mount(*rootReport);
    newRootPartition->mount(*rootReport);

    updatePartitionTable();

    page->setConfirmationMessage("Instalar o DelphinOS nas partições " + newBootPartition->deviceNode() + " e " + newRootPartition->deviceNode() + "?");
    page->setCanAdvance(true);
}