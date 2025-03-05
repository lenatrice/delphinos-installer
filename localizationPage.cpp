#include "keymaps.hpp"
#include "timezones.hpp"
#include "localizationPage.hpp"
#include <QLineEdit>

LocalizationPage::LocalizationPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Seja bem vindo ao instalador do DelphinOS",
        "Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável.\
Esse instalador irá lhe guiar por todas as etapas da instalação. Selecione seu idioma, layout do teclado e fuso-horário\
e clique em próximo para avançar para a próxima etapa.",
        640, 360
    );

    formLayout = new QFormLayout;
    formLayout->setHorizontalSpacing(25); 

    // Language option
    languageOptionCombobox = new QComboBox;
    languageOptionCombobox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    languageOptionCombobox->addItem("Português");
    languageOptionCombobox->addItem("English");

    // Keyboard layout and variant options
    keymapOptionLayout = new QHBoxLayout;
    keymapLayoutCombobox = new QComboBox;
    populateKeymapLayouts();

    keymapVariantCombobox = new QComboBox;

    updateKeymapLayout();
    connect(keymapLayoutCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this](const int)
        {
            keymapLayoutChanged = true;
            updateKeymapLayout();
            keymapLayoutChanged = false;
        }
    );
    connect(keymapVariantCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this](const int)
        {
            if (!keymapLayoutChanged) {
                updateKeymapVariant();
            }
        }
    );

    keymapTestBox = new QLineEdit;

    keymapOptionLayout->addWidget(keymapLayoutCombobox);
    keymapOptionLayout->addWidget(keymapVariantCombobox);

    // Timezone options
    timezoneCombobox = new QComboBox;
    populateTimezones();
    connect(timezoneCombobox, QOverload<const int>::of(&QComboBox::currentIndexChanged), this,
        [this](const int)
        {
            LocalizationPage::updateTimezone();
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

    formLayout->addRow(languageOptionLabel, languageOptionCombobox);
    formLayout->addRow("Layout do teclado:", keymapOptionLayout);
    formLayout->addRow(keymapTestLabel, keymapTestBox);
    formLayout->addRow(timezoneOptionLabel, timezoneCombobox);

    page->addStretch();
    page->addLayout(formLayout);
    page->addStretch();
}

void LocalizationPage::updateLanguage()
{

}

void LocalizationPage::populateKeymapLayouts()
{

    std::string layoutsPath = "/usr/share/X11/xkb/symbols/";
    QStringList layoutList;
    for (const auto &file : std::filesystem::directory_iterator(layoutsPath))
    {
        QString layoutFilename = QString::fromStdString(file.path().filename());
        if (getLayoutName(layoutFilename) != layoutFilename) {
            layoutList << getLayoutName(layoutFilename);
        }
        layoutList.sort(Qt::CaseInsensitive);
    }
    keymapLayoutCombobox->addItems(layoutList);
    QString currentKeymapLayout = getCurrentKeymapLayout();
    if (!currentKeymapLayout.isEmpty())
    {
        qDebug() << "Current keymap layout in use: " << currentKeymapLayout;
        if (keymapLayoutCombobox->findText(getLayoutName(currentKeymapLayout)))
        {
            keymapLayoutCombobox->setCurrentText(getLayoutName(currentKeymapLayout));
        }
    } else { 
        qWarning() << "Current keymap layout could not be determined";
    }
}

void LocalizationPage::updateKeymapLayout()
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

void LocalizationPage::updateKeymapVariant()
{
    QString layoutCode = getLayoutCode(keymapLayoutCombobox->currentText());
    QString variantCode = getVariantCode(keymapVariantCombobox->currentText());
    std::string setxkbmapCommand = "setxkbmap " + layoutCode.toStdString() + " -variant " + variantCode.toStdString();
    std::cout << "Running: " << setxkbmapCommand << std::endl;
    system(setxkbmapCommand.c_str());
}

void LocalizationPage::populateTimezones()
{
    timezoneCombobox->setInsertPolicy(QComboBox::InsertAtBottom);
    timezoneCombobox->addItem("Selecione o seu fuso-horário");
    for (auto i : timezoneList)
    {
        timezoneCombobox->addItem(i.first + " - " + i.second);
    }
    timezoneCombobox->setCurrentIndex(0);
}

void LocalizationPage::updateTimezone()
{
    if (timezoneCombobox->currentIndex() != 0 && timezoneCombobox->count() > timezoneList.size())
    {
        timezoneCombobox->removeItem(0);
        return;
    }

    QString selectedTimezone = timezoneList.at(timezoneCombobox->currentIndex()).first;
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
