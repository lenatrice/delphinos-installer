#include "mainWindow.hpp"
#include "keymapSearch.hpp"
#include <cstdlib>
#include <unistd.h>
#include <QLineEdit>
#include <vector>
#include <wait.h>
#include <timezones.hpp>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(640, 480);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Window layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);


    // Page content
    page = new QStackedWidget(this);
    pageContent1 = pageCreateLocalization();
    pageContent2 = pageCreatePartition();
    page->addWidget(pageContent1);
    page->addWidget(pageContent2);
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

QWidget * MainWindow::pageCreateLocalization()
{
    std::cout << "Running pageCreateLocalization()" << std::endl;

    QWidget *page = new QWidget(this);
    QVBoxLayout *pageLayout = new QVBoxLayout(page);

    PageTitle* title = new PageTitle("Seja bem vindo ao instalador do DelphinOS", this);
    pageLayout->addWidget(title);
    
    PageDescription* description = new PageDescription("Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável. \
Esse instalador irá lhe guiar por todas as etapas da instalação. Selecione seu idioma, layout do teclado e fuso-horário, e clique em próximo para avançar para a próxima etapa.", this);
    pageLayout->addWidget(description);

    // Language option
    QWidget* localizationFormWidget = new QWidget(this);
    QFormLayout* localizationFormLayout = new QFormLayout(localizationFormWidget);

    QComboBox* optionLanguageCombobox = new QComboBox(this);
    optionLanguageCombobox->addItem("Português");
    optionLanguageCombobox->addItem("English");

    // Keyboard layout and variant options
    QHBoxLayout* keymapOptionsLayout = new QHBoxLayout();
    QComboBox* optionKeymapLayoutCombobox = new QComboBox(this);
    populateKeymapLayouts(optionKeymapLayoutCombobox);

    QComboBox* optionKeymapVariantCombobox = new QComboBox(this);

    updateLayoutAndVariants(optionKeymapLayoutCombobox, optionKeymapVariantCombobox);
    connect(optionKeymapLayoutCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, optionKeymapLayoutCombobox, optionKeymapVariantCombobox](const int)
        {
            keymapLayoutChanged = true;
            updateLayoutAndVariants(optionKeymapLayoutCombobox, optionKeymapVariantCombobox);
            keymapLayoutChanged = false;
        }
    );
    connect(optionKeymapVariantCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, optionKeymapLayoutCombobox, optionKeymapVariantCombobox](const int)
        {
            if (!keymapLayoutChanged) {
                updateVariant(optionKeymapLayoutCombobox, optionKeymapVariantCombobox);
            }
        }
    );

    QLineEdit* keymapTest = new QLineEdit;

    keymapOptionsLayout->addWidget(optionKeymapLayoutCombobox);
    keymapOptionsLayout->addWidget(optionKeymapVariantCombobox);

    // Timezone options
    QComboBox* optionTimezoneCombobox = new QComboBox(this);
    populateTimezones(optionTimezoneCombobox);
    optionTimezoneCombobox->property("placeholderRemoved").toBool();
    optionTimezoneCombobox->setProperty("placeholderRemoved", false);
    connect(optionTimezoneCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this, optionTimezoneCombobox](const int optionTimezoneComboboxIndex)
        {
            MainWindow::updateTimezone(optionTimezoneCombobox, optionTimezoneComboboxIndex);
        }
    );
    localizationFormLayout->addRow("Idioma:", optionLanguageCombobox);
    localizationFormLayout->addRow("Layout do teclado:", keymapOptionsLayout);
    localizationFormLayout->addRow("Teste do teclado:", keymapTest);
    localizationFormLayout->addRow("Fuso-horário:", optionTimezoneCombobox);

    pageLayout->addWidget(localizationFormWidget);

    return page;
};

void MainWindow::populateKeymapLayouts(QComboBox* optionKeymapLayoutCombobox)
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
    optionKeymapLayoutCombobox->addItems(layoutList);
    std::string currentKeymapLayout = getCurrentKeymapLayout();
    if (!currentKeymapLayout.empty())
    {
        std::cout << "Current keymap layout in use: " << currentKeymapLayout.c_str() << std::endl;
        if (optionKeymapLayoutCombobox->findText(getLayoutName(QString::fromStdString(currentKeymapLayout))))
        {
            optionKeymapLayoutCombobox->setCurrentText(getLayoutName(QString::fromStdString(currentKeymapLayout)));
        }
    } else { 
        std::cout << "Current keymap layout could not be determined." << std::endl;
    }
}

void MainWindow::updateLayoutAndVariants(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox)
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

void MainWindow::updateVariant(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox)
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

QWidget * MainWindow::pageCreatePartition()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setSpacing(0);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    std::cout << "Running pageCreatePartition()" << std::endl;

    PageTitle* title = new PageTitle(this);
    pageLayout->addWidget(title);

    PageDescription* description = new PageDescription(this);
    pageLayout->addWidget(description);

    title->setText("Configurar partições para o sistema");
    description->setText("Nessa etapa iremos criar as partições que serão usadas pelo sistema ou selecionar partições já existentes.");

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
    int dragAreaHeight = 78;

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
    if (currentPage > 1)
    {
        currentPage--;
        page->setCurrentIndex(currentPage-1);
    }
}

void MainWindow::onNextClicked()
{
    if (currentPage < maxPages)
    {
        currentPage++;
        page->setCurrentIndex(currentPage-1);
    } else
    {
        close();
    }
}