#include "partitionPage.hpp"
#include <kpmcore/backend/corebackendmanager.h>
#include <kpmcore/backend/corebackend.h>
#include <kpmcore/core/device.h>
#include <kpmcore/core/partition.h>
#include <kpmcore/core/devicescanner.h>
#include <kpmcore/core/operationstack.h>
#include <QDebug>


PartitionPage::PartitionPage(QWidget* parent) : QWidget(parent)
{
    qDebug() << "Creating partition page";

    page = new PageContent(
        "Particionamento de discos",
        "Selecione o espaço livre no dispositivo em que deseja instalar o DelphinOS",
        640, 480
    );

    formLayout = new QFormLayout;
    QComboBox* deviceList = new QComboBox;
    formLayout->addRow("Dispositivos:", deviceList);

    page->addStretch();
    page->addLayout(formLayout);
    page->addStretch();

    QByteArray env = qgetenv("KPMCORE_BACKEND");
    auto backendName = env.isEmpty() ? CoreBackendManager::defaultBackendName() : env;

    qDebug() << "Loading backend:" << backendName;

    if (!CoreBackendManager::self()->load(backendName))
    {
        qWarning() << "Failed to load KPMCore backend" << backendName;
        return;
    };

    qDebug() << "Backend loaded successfully";

    /*CoreBackend* backend = CoreBackendManager::self()->backend();*/

    OperationStack* operationStack = new OperationStack(this);
    DeviceScanner* deviceScanner = new DeviceScanner(this, *operationStack);
    deviceScanner->scan();

    QList<Device*> devices = operationStack->previewDevices();

    for (Device* device : devices)
    {
        deviceList->addItem(device->deviceNode());
    }

}
