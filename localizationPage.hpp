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

class LocalizationPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;

    // Language option
    QComboBox* languageOptionCombobox;

    // Keymap options
    QHBoxLayout* keymapOptionLayout;
    QComboBox* keymapLayoutCombobox;
    QComboBox* keymapVariantCombobox;
    QLineEdit* keymapTestBox;

    // Timezone option
    QComboBox* timezoneCombobox;
    
    void populateKeymapLayouts();
    void populateTimezones();
    bool keymapLayoutChanged = false;
    
private slots:
    void updateLanguage();
    void updateKeymapLayout();
    void updateKeymapVariant();
    void updateTimezone();

public: 
    PageContent* getPage()
    {
        return page;
    };

    LocalizationPage(QWidget* parent);
};
