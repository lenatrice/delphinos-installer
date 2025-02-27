#include <iostream>
#include <filesystem>
#include <fstream>
#include <QString>
#include <QMap>
#include <regex>
#include <QXmlStreamReader>

QString getLayoutName(const QString &layout)
{
    static const QMap<QString, QString> languageMap
    {
        {"al", "Shqip"},                   // Albanian
        {"et", "አማርኛ"},                // Amharic
        {"am", "Հայերեն"},             // Armenian
        {"ara", "العربية"},            // Arabic
        {"eg", "العربية (مصر)"},       // Arabic (Egypt)
        {"iq", "العربية (العراق)"},    // Arabic (Iraq)
        {"ma", "العربية (المغرب)"},    // Arabic (Morocco)
        {"sy", "العربية (سوريا)"},     // Arabic (Syria)
        {"az", "Azərbaycan dili"},     // Azerbaijani
        {"ml", "Bamanankan"},          // Bambara
        {"bd", "বাংলা"},               // Bangla
        {"by", "Беларуская"},          // Belarusian
        {"be", "Nederlands (België)"}, // Belgian (Dutch)
        {"dz", "Tamaziɣt"},            // Berber (Algeria, Latin)
        {"ba", "Bosanski"},            // Bosnian
        {"brai", "Braille"},           // Braille
        {"bg", "Български"},           // Bulgarian
        {"mm", "မြန်မာ"},               // Burmese
        {"cn", "中文"},                // Chinese
        {"hr", "Hrvatski"},            // Croatian
        {"cz", "Čeština"},             // Czech
        {"dk", "Dansk"},               // Danish
        {"af", "دری"},                 // Dari
        {"mv", "ދިވެހި"},                 // Dhivehi
        {"nl", "Nederlands"},          // Dutch
        {"bt", "རྫོང་ཁ"},                // Dzongkha
        {"au", "English (Australia)"},
        {"cm", "English (Cameroon)"},
        {"gh", "English (Ghana)"},
        {"nz", "English (New Zealand)"},
        {"ng", "English (Nigeria)"},
        {"za", "English (South Africa)"},
        {"gb", "English (UK)"},
        {"us", "English (US)"},
        {"epo", "Esperanto"},
        {"ee", "Eesti"},    // Estonian
        {"fo", "Føroyskt"}, // Faroese
        {"ph", "Filipino"},
        {"fi", "Suomi"},    // Finnish
        {"fr", "Français"}, // French
        {"ca", "Français (Canada)"},
        {"cd", "Français (République Démocratique du Congo)"},
        {"tg", "Français (Togo)"},
        {"ge", "ქართული"}, // Georgian
        {"de", "Deutsch"}, // German
        {"at", "Deutsch (Österreich)"},
        {"ch", "Deutsch (Schweiz)"},
        {"gr", "Ελληνικά"},         // Greek
        {"il", "עברית"},            // Hebrew
        {"hu", "Magyar"},           // Hungarian
        {"is", "Íslenska"},         // Icelandic
        {"in", "हिन्दी"},            // Indian (Hindi)
        {"id", "Bahasa Indonesia"}, // Indonesian (Latin)
        {"ie", "Gaeilge"},          // Irish
        {"it", "Italiano"},         // Italian
        {"jp", "日本語"},           // Japanese
        {"kz", "Қазақша"},          // Kazakh
        {"kh", "ខ្មែរ"},             // Khmer (Cambodia)
        {"kr", "한국어"},           // Korean
        {"kg", "Кыргызча"},         // Kyrgyz
        {"la", "ລາວ"},              // Lao
        {"lv", "Latviešu"},         // Latvian
        {"lt", "Lietuvių"},         // Lithuanian
        {"mk", "Македонски"},       // Macedonian
        {"my", "بهاس ملايو"},       // Malay (Jawi, Arabic Keyboard)
        {"mt", "Malti"},            // Maltese
        {"md", "Moldovenească"},    // Moldavian
        {"mn", "Монгол"},           // Mongolian
        {"me", "Crnogorski"},       // Montenegrin
        {"np", "नेपाली"},            // Nepali
        {"gn", "N'Ko"},             // N'Ko (AZERTY)
        {"no", "Norsk"},            // Norwegian
        {"ir", "فارسی"},            // Persian
        {"pl", "Polski"},           // Polish
        {"pt", "Português (Portugal)"},        // Portuguese
        {"br", "Português (Brasil)"},
        {"ro", "Română"},      // Romanian
        {"ru", "Русский"},     // Russian
        {"rs", "Српски"},      // Serbian
        {"lk", "සිංහල"},        // Sinhala (phonetic)
        {"sk", "Slovenčina"},  // Slovak
        {"si", "Slovenščina"}, // Slovenian
        {"es", "Español"},     // Spanish
        {"latam", "Español (Latinoamérica)"},
        {"ke", "Kiswahili (Kenya)"},
        {"tz", "Kiswahili (Tanzania)"},
        {"se", "Svenska"},        // Swedish
        {"tw", "繁體中文"},       // Taiwanese
        {"tj", "Тоҷикӣ"},         // Tajik
        {"th", "ไทย"},            // Thai
        {"tr", "Türkçe"},         // Turkish
        {"ua", "Українська"},     // Ukrainian
        {"pk", "اردو (پاکستان)"} // Urdu (Pakistan)
    };
    return languageMap.value(layout, layout);
}

QString getLayoutCode(const QString &layout)
{
    static const QMap<QString, QString> languageMap
    {
        {"al", "Shqip"},                   // Albanian
        {"et", "አማርኛ"},                // Amharic
        {"am", "Հայերեն"},             // Armenian
        {"ara", "العربية"},            // Arabic
        {"eg", "العربية (مصر)"},       // Arabic (Egypt)
        {"iq", "العربية (العراق)"},    // Arabic (Iraq)
        {"ma", "العربية (المغرب)"},    // Arabic (Morocco)
        {"sy", "العربية (سوريا)"},     // Arabic (Syria)
        {"az", "Azərbaycan dili"},     // Azerbaijani
        {"ml", "Bamanankan"},          // Bambara
        {"bd", "বাংলা"},               // Bangla
        {"by", "Беларуская"},          // Belarusian
        {"be", "Nederlands (België)"}, // Belgian (Dutch)
        {"dz", "Tamaziɣt"},            // Berber (Algeria, Latin)
        {"ba", "Bosanski"},            // Bosnian
        {"brai", "Braille"},           // Braille
        {"bg", "Български"},           // Bulgarian
        {"mm", "မြန်မာ"},               // Burmese
        {"cn", "中文"},                // Chinese
        {"hr", "Hrvatski"},            // Croatian
        {"cz", "Čeština"},             // Czech
        {"dk", "Dansk"},               // Danish
        {"af", "دری"},                 // Dari
        {"mv", "ދިވެހި"},                 // Dhivehi
        {"nl", "Nederlands"},          // Dutch
        {"bt", "རྫོང་ཁ"},                // Dzongkha
        {"au", "English (Australia)"},
        {"cm", "English (Cameroon)"},
        {"gh", "English (Ghana)"},
        {"nz", "English (New Zealand)"},
        {"ng", "English (Nigeria)"},
        {"za", "English (South Africa)"},
        {"gb", "English (UK)"},
        {"us", "English (US)"},
        {"epo", "Esperanto"},
        {"ee", "Eesti"},    // Estonian
        {"fo", "Føroyskt"}, // Faroese
        {"ph", "Filipino"},
        {"fi", "Suomi"},    // Finnish
        {"fr", "Français"}, // French
        {"ca", "Français (Canada)"},
        {"cd", "Français (République Démocratique du Congo)"},
        {"tg", "Français (Togo)"},
        {"ge", "ქართული"}, // Georgian
        {"de", "Deutsch"}, // German
        {"at", "Deutsch (Österreich)"},
        {"ch", "Deutsch (Schweiz)"},
        {"gr", "Ελληνικά"},         // Greek
        {"il", "עברית"},            // Hebrew
        {"hu", "Magyar"},           // Hungarian
        {"is", "Íslenska"},         // Icelandic
        {"in", "हिन्दी"},            // Indian (Hindi)
        {"id", "Bahasa Indonesia"}, // Indonesian (Latin)
        {"ie", "Gaeilge"},          // Irish
        {"it", "Italiano"},         // Italian
        {"jp", "日本語"},           // Japanese
        {"kz", "Қазақша"},          // Kazakh
        {"kh", "ខ្មែរ"},             // Khmer (Cambodia)
        {"kr", "한국어"},           // Korean
        {"kg", "Кыргызча"},         // Kyrgyz
        {"la", "ລາວ"},              // Lao
        {"lv", "Latviešu"},         // Latvian
        {"lt", "Lietuvių"},         // Lithuanian
        {"mk", "Македонски"},       // Macedonian
        {"my", "بهاس ملايو"},       // Malay (Jawi, Arabic Keyboard)
        {"mt", "Malti"},            // Maltese
        {"md", "Moldovenească"},    // Moldavian
        {"mn", "Монгол"},           // Mongolian
        {"me", "Crnogorski"},       // Montenegrin
        {"np", "नेपाली"},            // Nepali
        {"gn", "N'Ko"},             // N'Ko (AZERTY)
        {"no", "Norsk"},            // Norwegian
        {"ir", "فارسی"},            // Persian
        {"pl", "Polski"},           // Polish
        {"pt", "Português (Portugal)"},        // Portuguese
        {"br", "Português (Brasil)"},
        {"ro", "Română"},      // Romanian
        {"ru", "Русский"},     // Russian
        {"rs", "Српски"},      // Serbian
        {"lk", "සිංහල"},        // Sinhala (phonetic)
        {"sk", "Slovenčina"},  // Slovak
        {"si", "Slovenščina"}, // Slovenian
        {"es", "Español"},     // Spanish
        {"latam", "Español (Latinoamérica)"},
        {"ke", "Kiswahili (Kenya)"},
        {"tz", "Kiswahili (Tanzania)"},
        {"se", "Svenska"},        // Swedish
        {"tw", "繁體中文"},       // Taiwanese
        {"tj", "Тоҷикӣ"},         // Tajik
        {"th", "ไทย"},            // Thai
        {"tr", "Türkçe"},         // Turkish
        {"ua", "Українська"},     // Ukrainian
        {"pk", "اردو (پاکستان)"} // Urdu (Pakistan)
    };
    return languageMap.key(layout, layout);
}


QString getVariantName(const QString &variant)
{
    if (variant == "abnt2")
        return "ABNT2";
    if (variant == "intl")
        return "International";
    if (variant == "nodeadkeys")
        return "No Dead Keys";
    if (variant == "latin")
        return "Latin";
    if (variant == "latin9")
        return "Latin9";
    if (variant == "dvorak")
        return "Dvorak";
    if (variant == "nativo")
        return "Nativo";
    if (variant == "basic")
        return "Basic";
    if (variant == "cyrillic")
        return "Cyrillic";
    if (variant == "legacy")
        return "Legacy";
    if (variant == "qwerty")
        return "QWERTY";
    if (variant == "qwertz")
        return "QWERTZ";
    if (variant == "azerty")
        return "AZERTY";
    if (variant == "arabic")
        return "Arabic";
    if (variant == "ergonomic")
        return "Ergonomic";
    if (variant == "ru")
        return "Russian";
    if (variant == "phonetic")
        return "Phonetic";
    if (variant == "biblical")
        return "Biblical";
    if (variant == "indigenous")
        return "Indigenous";
    return variant;
}

QStringList listLayoutVariants(const std::string &layout)
{
    std::string path = "/usr/share/X11/xkb/symbols/" + layout;
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "ERRO: não foi possível abrir o arquivo " << path << std::endl;
        return QStringList();

    }
    std::regex variantRegex("^\\s*xkb_symbols\\s*\"([^\"]+)\"");
    std::string line;

    QStringList variantList;

    while (std::getline(file, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, variantRegex)) {
            variantList << QString::fromStdString(match[1]);
        }
    }

    file.close();

    return variantList;
}

QString getKeymapLayoutDefaultVariant(const QString& keymapLayout)
{
    std::ifstream file("/usr/share/X11/xkb/symbols/" + keymapLayout.toStdString());
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 0;
    }

    std::string line;
    bool foundDefault = false;
    std::regex pattern("xkb_symbols\\s+\"([^\"]+)\"");
    std::smatch match;

    while (std::getline(file, line)) {
        if (foundDefault)
        {
            if (std::regex_search(line, match, pattern)) {
                return QString::fromStdString(match[1].str());
            }
        }

        if (line.find("default")) {
            foundDefault = true;
        }
    }

    file.close();
    return 0;
}