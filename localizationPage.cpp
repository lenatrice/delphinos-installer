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

#include "keymaps.hpp"
#include "timezones.hpp"
#include "localizationPage.hpp"
#include <QLineEdit>

LocalizationPage::LocalizationPage(QWidget* parent) : QWidget(parent)
{
    page = new PageContent(
        "Seja bem vindo ao instalador do DelphinOS",
        "Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável. \
Esse instalador irá lhe guiar por todas as etapas da instalação. Selecione seu idioma, layout do teclado e fuso-horário \
e clique em próximo para avançar para a próxima etapa.",
        640, 360, this
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

    connect(keymapLayoutCombobox, &QComboBox::currentIndexChanged, this,
        [this](const int)
        {
            keymapLayoutChanged = true;
            updateKeymapLayout();
            keymapLayoutChanged = false;
        }
    );

    connect(keymapVariantCombobox, &QComboBox::currentIndexChanged, this,
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
    keymapOptionLayout->setSpacing(4);

    // Timezone options
    timezoneCombobox = new QComboBox;
    populateTimezones();
    connect(timezoneCombobox, &QComboBox::currentIndexChanged, this,
        [this](const int)
        {
            LocalizationPage::updateTimezone();
        }
    );

    formLayout->addRow("Idioma:", languageOptionCombobox);
    formLayout->addRow("Layout do teclado:", keymapOptionLayout);
    formLayout->addRow("Teste do teclado:", keymapTestBox);
    formLayout->addRow("Fuso-horário:", timezoneCombobox);

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
        qDebug() << "Default keyboard variant: " << keymapVariantName.toStdString();
    } else {
        qDebug() << "Default keyboard layout variant not found." << keymapVariantName.toStdString();
    }

    std::string setxkbmapCommand = "setxkbmap " + layoutCode.toStdString() + " -variant " + defaultKeymapVariant.toStdString();
    qDebug() << "Running: " << setxkbmapCommand;
    system(setxkbmapCommand.c_str());

    return;
}

void LocalizationPage::updateKeymapVariant()
{
    QString layoutCode = getLayoutCode(keymapLayoutCombobox->currentText());
    QString variantCode = getVariantCode(keymapVariantCombobox->currentText());
    std::string setxkbmapCommand = "setxkbmap " + layoutCode.toStdString() + " -variant " + variantCode.toStdString();
    qDebug() << "Running: " << setxkbmapCommand;
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
    qDebug() << "Selected timezone: " + selectedTimezone.toStdString();

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
