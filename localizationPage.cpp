#include "keymaps.hpp"
#include "timezones.hpp"
#include "mainWindow.hpp"
#include <QLineEdit>


QWidget* MainWindow::pageCreateLocalization()
{
    WindowPage* page = new WindowPage(
        "Seja bem vindo ao instalador do DelphinOS",
        "Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável.\
Esse instalador irá lhe guiar por todas as etapas da instalação. Selecione seu idioma, layout do teclado e fuso-horário\
e clique em próximo para avançar para a próxima etapa."
    );

    QWidget* localizationWidget = new QWidget(this);
    QFormLayout* localizationFormLayout = new QFormLayout(localizationWidget);
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


    page->addWidget(localizationWidget);

    return page;
}

void MainWindow::updateLanguage()
{

}

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
