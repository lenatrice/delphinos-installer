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
