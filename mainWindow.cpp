#include "mainWindow.hpp"
#include "keymapSearch.hpp"
#include "timezones.hpp"
#include "network.hpp"
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <NetworkManager.h>

#define signals Q_SIGNALs

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(640, 480);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Window layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignTop);

    // Page content
    page = new QStackedWidget(this);
    pageContent[0] = pageCreateLocalization();
    pageContent[1] = pageCreateNetwork();
    pageContent[2] = pageCreatePartition();
    page->addWidget(pageContent[0]);
    page->addWidget(pageContent[1]);
    page->addWidget(pageContent[2]);
    page->setCurrentIndex(0);

    mainLayout->addWidget(page);

    // Navigation buttons
    QWidget * navigationWidget = new QWidget(this);
    QHBoxLayout * navigationButtonLayout = new QHBoxLayout(navigationWidget);

    buttonBack = new QPushButton("Voltar", this);
    navigationButtonLayout->addWidget(buttonBack);

    buttonNext = new QPushButton("Avançar", this);
    navigationButtonLayout->addWidget(buttonNext);

    navigationButtonLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    mainLayout->addWidget(navigationWidget);


    connect(buttonBack, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(buttonNext, &QPushButton::clicked, this, &MainWindow::onNextClicked);
};

QWidget* MainWindow::pageCreateLocalization()
{
    WindowPage* page = new WindowPage(
        "Seja bem vindo ao instalador do DelphinOS",
        "Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável.\
Esse instalador irá lhe guiar por todas as etapas da instalação. Selecione seu idioma, layout do teclado e fuso-horário\
e clique em próximo para avançar para a próxima etapa."
    );

    QWidget* localizationFormWidget = new QWidget(this);
    QFormLayout* localizationFormLayout = new QFormLayout(localizationFormWidget);
    localizationFormLayout->setHorizontalSpacing(25); 

    // Language option
    QComboBox* languageOptionCombobox = new QComboBox(this);
    languageOptionCombobox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    languageOptionCombobox->addItem("Português");
    languageOptionCombobox->addItem("English");

    // Keyboard layout and variant options
    QHBoxLayout* keymapOptionLayout = new QHBoxLayout();
    QComboBox* keymapLayoutCombobox = new QComboBox(this);
    populateKeymapLayouts(keymapLayoutCombobox);

    QComboBox* keymapVariantCombobox = new QComboBox(this);

    updateKeymapLayout(keymapLayoutCombobox, keymapVariantCombobox);
    connect(keymapLayoutCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, keymapLayoutCombobox, keymapVariantCombobox](const int)
        {
            keymapLayoutChanged = true;
            updateKeymapLayout(keymapLayoutCombobox, keymapVariantCombobox);
            keymapLayoutChanged = false;
        }
    );
    connect(keymapVariantCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, keymapLayoutCombobox, keymapVariantCombobox](const int)
        {
            if (!keymapLayoutChanged) {
                updateKeymapVariant(keymapLayoutCombobox, keymapVariantCombobox);
            }
        }
    );

    QLineEdit* keymapTestBox = new QLineEdit;

    keymapOptionLayout->addWidget(keymapLayoutCombobox);
    keymapOptionLayout->addWidget(keymapVariantCombobox);

    // Timezone options
    QComboBox* timezoneCombobox = new QComboBox(this);
    populateTimezones(timezoneCombobox);
    timezoneCombobox->property("placeholderRemoved").toBool();
    timezoneCombobox->setProperty("placeholderRemoved", false);
    connect(timezoneCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, timezoneCombobox](const int timezoneComboboxIndex)
        {
            MainWindow::updateTimezone(timezoneCombobox, timezoneComboboxIndex);
        }
    );

    QLabel* languageOptionLabel = new QLabel("Idioma:");
    languageOptionLabel->setMinimumWidth(languageOptionLabel->sizeHint().width());

    QLabel* keymapOptionLabel = new QLabel("Layout do teclado:");
    keymapOptionLabel->setMinimumWidth(keymapOptionLabel->sizeHint().width());

    QLabel* keymapTestLabel = new QLabel("Teste do teclado:");
    keymapTestLabel->setMinimumWidth(keymapTestLabel->sizeHint().width());

    QLabel* timezoneOptionLabel = new QLabel("Fuso-horário:");
    timezoneOptionLabel->setMinimumWidth(timezoneOptionLabel->sizeHint().width());

    localizationFormLayout->addRow(languageOptionLabel, languageOptionCombobox);
    localizationFormLayout->addRow(keymapOptionLabel, keymapOptionLayout);
    localizationFormLayout->addRow(keymapTestLabel, keymapTestBox);
    localizationFormLayout->addRow(timezoneOptionLabel, timezoneCombobox);


    page->addWidget(localizationFormWidget);

    return page;
};

void MainWindow::populateKeymapLayouts(QComboBox* keymapOptionCombobox)
{
    std::string layoutsPath = "/usr/share/X11/xkb/symbols/";
    QStringList layoutList;
    for (const auto &file : std::filesystem::directory_iterator(layoutsPath))
    {
        std::string layoutFilename = file.path().filename();
        QString layoutFilenameQStr = QString::fromStdString(layoutFilename);
        if (getLayoutName(layoutFilenameQStr) != layoutFilenameQStr) {
            layoutList << getLayoutName(layoutFilenameQStr);
        }
        layoutList.sort(Qt::CaseInsensitive);
    }
    keymapOptionCombobox->addItems(layoutList);
    std::string currentKeymapLayout = getCurrentKeymapLayout();
    if (!currentKeymapLayout.empty())
    {
        std::cout << "Current keymap layout in use: " << currentKeymapLayout.c_str() << std::endl;
        if (keymapOptionCombobox->findText(getLayoutName(QString::fromStdString(currentKeymapLayout))))
        {
            keymapOptionCombobox->setCurrentText(getLayoutName(QString::fromStdString(currentKeymapLayout)));
        }
    } else { 
        std::cout << "Current keymap layout could not be determined." << std::endl;
    }
}

void MainWindow::updateKeymapLayout(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox)
{
    QString layoutCode = getLayoutCode(keymapLayoutCombobox->currentText());

    QStringList keymapVariantList = listLayoutVariants(layoutCode.toStdString());
    keymapVariantCombobox->clear();

    for (QString iterator : keymapVariantList)
    {
        keymapVariantCombobox->addItem(getVariantName(iterator));
    }

    QString defaultKeymapVariant = getKeymapLayoutDefaultVariant(layoutCode);
    QString keymapVariantName=getVariantName(defaultKeymapVariant);

    if (keymapVariantCombobox->findText(keymapVariantName) != -1)
    {
        keymapVariantCombobox->setCurrentText(keymapVariantName);
        std::cout << "Default keyboard variant: " << keymapVariantName.toStdString() << std::endl;
    } else {
        std::cout << "Default keyboard layout variant not found." << keymapVariantName.toStdString() << std::endl;
    }

    std::string setxkbmapCommand = "setxkbmap " + layoutCode.toStdString() + " -variant " + defaultKeymapVariant.toStdString();
    std::cout << "Running: " << setxkbmapCommand << std::endl;
    system(setxkbmapCommand.c_str());

    return;
}

void MainWindow::updateKeymapVariant(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox)
{
    QString layoutCode = getLayoutCode(keymapLayoutCombobox->currentText());
    QString variantCode = getVariantCode(keymapVariantCombobox->currentText());
    std::string setxkbmapCommand = "setxkbmap " + layoutCode.toStdString() + " -variant " + variantCode.toStdString();
    std::cout << "Running: " << setxkbmapCommand << std::endl;
    system(setxkbmapCommand.c_str());
}

void MainWindow::populateTimezones(QComboBox* timezoneCombobox)
{
    timezoneCombobox->setInsertPolicy(QComboBox::InsertAtBottom);
    timezoneCombobox->addItem("Selecione o seu fuso-horário");
    for (auto i : timezoneList)
    {
        timezoneCombobox->addItem(i.first + " - " + i.second);
    }
    timezoneCombobox->setCurrentIndex(0);
}

void MainWindow::updateTimezone(QComboBox* timezoneCombobox, int index)
{
    if (index != 0 && timezoneCombobox->count() > timezoneList.size())
    {
        timezoneCombobox->removeItem(0);
        return;
    }

    QString selectedTimezone = timezoneList.at(index).first;
    std::cout << "Selected timezone: " + selectedTimezone.toStdString() << std::endl;

    // Invert + and - symbols for UTC model
    for (QChar &ch : selectedTimezone) {
        if (ch == '+') {
            ch = '-';
        } else if (ch == '-') {
            ch = '+';
        }
    };
    system("qt-sudo timedatectl set-timezone Etc/" + selectedTimezone.toUtf8());
};

QWidget* MainWindow::pageCreateNetwork()
{
    WindowPage* page = new WindowPage(
        "Configurar conexão de rede",
        "Selecione a rede que deseja utilizar para instalar o sistema."
    );

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

    if (!dbusInterface.isValid())
    {
        qCritical() << "Failed to connect to D-Bus: " << QDBusConnection::systemBus().lastError().message();
        return page;
    }

    QDBusReply<QList<QDBusObjectPath>> dbusReply = dbusInterface.call("GetAllDevices");

    if (!dbusReply.isValid())
    {
        qCritical() << "Failed to call D-Bus method: " << dbusReply.error().message();
        return page;
    }

    QList<QDBusObjectPath> dbusDevices = dbusReply.value();

    qDebug() << "Found D-Bus devices:";

    QWidget* networkFormWidget = new QWidget(this);
    QFormLayout* networkFormLayout = new QFormLayout(networkFormWidget);
    networkFormLayout->setAlignment(Qt::AlignTop);

    QListWidget* networkDeviceList = new QListWidget(this);
    networkDeviceList->setMaximumHeight(120);
    networkFormLayout->addRow("Dispositivos de rede: ", networkDeviceList);

    for (const QDBusObjectPath& dbusDevicePath : dbusDevices)
    {
        QDBusInterface dbusDeviceInterface("org.freedesktop.NetworkManager", dbusDevicePath.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QVariant> dbusDeviceReply = dbusDeviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "Interface");
        if (dbusDeviceReply.isValid())
        {
            QString deviceName = dbusDeviceReply.value().toString();
            QDBusReply<QVariant> deviceTypeReply = dbusDeviceInterface.call("Get", "org.freedesktop.NetworkManager.Device", "DeviceType");
            if (deviceTypeReply.isValid())
            {
                int deviceType = deviceTypeReply.value().toInt();
                deviceName += " (" + networkTypeMap.value(deviceType, "Unknown") + ")";
            } else {
                qWarning() << "Coult not determine type of device " << deviceName << ": " << deviceTypeReply.error().message();
            }

            QListWidgetItem* newDevice = new QListWidgetItem(deviceName);
            newDevice->setData(networkDevicePathRole, dbusDevicePath.path());
            networkDeviceList->addItem(newDevice);
        } else {
            qCritical() << "Failed to get device type for path " << dbusDevicePath.path() << ":" << dbusDeviceReply.error().message();
        }
    }

    connect(networkDeviceList, &QListWidget::currentItemChanged, [this](QListWidgetItem* item) {
            QString dbusPath = item->data(networkDevicePathRole).toString();
            qDebug() << "Selected device D-Bus path: " << dbusPath;
        }
    );



    page->addWidget(networkFormWidget);

    return page;
};

QWidget* MainWindow::pageCreatePartition()
{
    WindowPage* page = new WindowPage(
        "Configurar partições para o sistema",
        "Nessa etapa iremos criar as partições que serão usadas pelo sistema ou selecionar partições já existentes."
    );

    return page;
};

// Custom window background 
void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen backgroundBorder(Qt::black);
    backgroundBorder.setWidth(1);
    painter.setPen(backgroundBorder);

    QBrush brush(QColor("#191B21"));
    painter.setBrush(brush);

    QRectF background(0, 0, this->width(), this->height());
    painter.drawRoundedRect(background, 20, 20);

    QMainWindow::paintEvent(event);
}

// Allows Mouse to drag window on the top
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    int dragAreaHeight = 64;

    if (event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight)
    {
        dragging = true;
        dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        event->accept();
    }
    else
    {
        QMainWindow::mousePressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging == true && event->buttons() & Qt::LeftButton)
    {
        this->move(event->globalPos() - dragPosition);
        event->accept();
    }
    else
    {
        QMainWindow::mouseMoveEvent(event);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && dragging == true)
    {
        dragging = false;
        event->accept();
    }
    else
    {
        QMainWindow::mouseReleaseEvent(event);
    }
}

// Window buttons functions

void MainWindow::onBackClicked()
{
    if (page->currentIndex() > 0)
    {
        page->setCurrentIndex(page->currentIndex()-1);
    }
}

void MainWindow::onNextClicked()
{
    if (page->currentIndex() != page->count() - 1)
    {
        page->setCurrentIndex(page->currentIndex()+1);
    } else
    {
        close();
    }
}