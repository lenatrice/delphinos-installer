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

#include <iostream>
#include <filesystem>
#include <fstream>
#include <QString>
#include <QMap>
#include <regex>
#include <cstdlib>
#include <array>
#include <memory>

static const QMap<QString, QString> layoutMap
{
    {"al", "Shqip" },               // Albanian
    {"et", "አማርኛ" },               // Amharic
    {"am", "Հայերեն" },              // Armenian
    {"ara", "العربية" },            // Arabic
    {"eg", "العربية (مصر)" },       // Arabic (Egypt)
    {"iq", "العربية (العراق)" },    // Arabic (Iraq)
    {"ma", "العربية (المغرب)" },    // Arabic (Morocco)
    {"sy", "العربية (سوريا)" },     // Arabic (Syria)
    {"az", "Azərbaycan dili" },     // Azerbaijani
    {"ml", "Bamanankan" },          // Bambara
    {"bd", "বাংলা" },                 // Bangla
    {"by", "Беларуская" },          // Belarusian
    {"be", "Nederlands (België)" }, // Belgian (Dutch)
    {"dz", "Tamaziɣt" },            // Berber (Algeria, Latin)
    {"ba", "Bosanski" },            // Bosnian
    {"brai", "Braille" },           // Braille
    {"bg", "Български" },           // Bulgarian
    {"mm", "မြန်မာ" },                // Burmese
    {"cn", "中文" },                  // Chinese
    {"hr", "Hrvatski" },            // Croatian
    {"cz", "Čeština" },             // Czech
    {"dk", "Dansk" },               // Danish
    {"af", "دری" },                 // Dari
    {"mv", "ދިވެހި" },                 // Dhivehi
    {"nl", "Nederlands" },          // Dutch
    {"bt", "རྫོང་ཁ" },                // Dzongkha
    {"au", "English (Australia)" },
    {"cm", "English (Cameroon)" },
    {"gh", "English (Ghana)" },
    {"nz", "English (New Zealand)" },
    {"ng", "English (Nigeria)" },
    {"za", "English (South Africa)" },
    {"gb", "English (UK)" },
    {"us", "English (US)" },
    {"epo", "Esperanto" },
    {"ee", "Eesti" },               // Estonian
    {"fo", "Føroyskt" },            // Faroese
    {"ph", "Filipino" },
    {"fi", "Suomi" },               // Finnish
    {"fr", "Français" },            // French
    {"ca", "Français (Canada)" },
    {"cd", "Français (République Démocratique du Congo)" },
    {"tg", "Français (Togo)" },
    {"ge", "ქართული" },             // Georgian
    {"de", "Deutsch" },             // German
    {"at", "Deutsch (Österreich)" },
    {"ch", "Deutsch (Schweiz)" },
    {"gr", "Ελληνικά" },            // Greek
    {"il", "עברית" },                // Hebrew
    {"hu", "Magyar" },              // Hungarian
    {"is", "Íslenska" },            // Icelandic
    {"in", "हिन्दी" },                 // Indian (Hindi)
    {"id", "Bahasa Indonesia" },    // Indonesian (Latin)
    {"ie", "Gaeilge" },             // Irish
    {"it", "Italiano" },            // Italian
    {"jp", "日本語" },                 // Japanese
    {"kz", "Қазақша" },             // Kazakh
    {"kh", "ខ្មែរ" },                  // Khmer (Cambodia)
    {"kr", "한국어" },                 // Korean
    {"kg", "Кыргызча" },            // Kyrgyz
    {"la", "ລາວ" },                 // Lao
    {"lv", "Latviešu" },            // Latvian
    {"lt", "Lietuvių" },            // Lithuanian
    {"mk", "Македонски" },          // Macedonian
    {"my", "بهاس ملايو" },           // Malay (Jawi, Arabic Keyboard)
    {"mt", "Malti" },               // Maltese
    {"md", "Moldovenească" },       // Moldavian
    {"mn", "Монгол" },              // Mongolian
    {"me", "Crnogorski" },          // Montenegrin
    {"np", "नेपाली" },                // Nepali
    {"gn", "N'Ko" },                // N'Ko (AZERTY)
    {"no", "Norsk" },               // Norwegian
    {"ir", "فارسی" },               // Persian
    {"pl", "Polski" },              // Polish
    {"pt", "Português (Portugal)" },        // Portuguese
    {"br", "Português (Brasil)" },
    {"ro", "Română" },              // Romanian
    {"ru", "Русский" },             // Russian
    {"rs", "Српски" },              // Serbian
    {"lk", "සිංහල" },                // Sinhala (phonetic)
    {"sk", "Slovenčina" },          // Slovak
    {"si", "Slovenščina" },         // Slovenian
    {"es", "Español" },             // Spanish
    {"latam", "Español (Latinoamérica)" },
    {"ke", "Kiswahili (Kenya)" },
    {"tz", "Kiswahili (Tanzania)" },
    {"se", "Svenska" },             // Swedish
    {"tw", "繁體中文" },                // Taiwanese
    {"tj", "Тоҷикӣ" },              // Tajik
    {"th", "ไทย" },                  // Thai
    {"tr", "Türkçe" },              // Turkish
    {"ua", "Українська" },          // Ukrainian
    {"pk", "اردو (پاکستان)" }       // Urdu (Pakistan)
};

static const QMap<QString, QString> variantMap
{
    { "abnt2", "ABNT2" },
    { "intl", "International" },
    { "alt-intl", "Alternative International" },
    { "nodeadkeys", "No Dead Keys" },
    { "euro", "With Euro Symbol" },
    { "standard", "Standard" },
    { "mac", "Mac" },
    { "mac_nodeadkeys", "Mac (No Dead Keys)" },
    { "mac-nodeadkeys", "Mac (No Dead Keys)" },
    { "lekp",	"Lekp" },
    { "lekpa",	"Lekpa" },
    { "sgs",	"SGS" },
    { "ratise",	"Ratise" },
    { "us_dvorak",	"US (Dvorak)" },
    { "acc", "With Accents" },
    { "bksl", "With Backslash" },
    { "qwerty_bksl", "QWERTY (with Backslash)" },
    { "101_qwertz_comma_dead",	"101 QWERTZ (Comma Dead Key)" },
    { "101_qwertz_comma_nodead",	"101 QWERTZ (Comma Non-Dead Key)" },
    { "101_qwertz_dot_dead",	"101 QWERTZ (Dot Dead Key)" },
    { "101_qwertz_dot_nodead",	"101 QWERTZ (Dot Non-Dead Key)" },
    { "101_qwerty_comma_dead",	"101 QWERTY (Comma Dead Key)" },
    { "101_qwerty_comma_nodead",	"101 QWERTY (Comma Non-Dead Key)" },
    { "101_qwerty_dot_dead",	"101 QWERTY (Dot Dead Key)" },
    { "101_qwerty_dot_nodead",	"101 QWERTY (Dot Non-Dead Key)" },
    { "102_qwertz_comma_dead",	"102 QWERTZ (Comma Dead Key)" },
    { "102_qwertz_comma_nodead",	"102 QWERTZ (Comma Non-Dead Key)" },
    { "102_qwertz_dot_dead",	"102 QWERTZ (Dot Dead Key)" },
    { "102_qwertz_dot_nodead",	"102 QWERTZ (Dot Non-Dead Key)" },
    { "102_qwerty_comma_dead",	"102 QWERTY (Comma Dead Key)" },
    { "102_qwerty_comma_nodead",	"102 QWERTY (Comma Non-Dead Key)" },
    { "102_qwerty_dot_dead",	"102 QWERTY (Dot Dead Key)" },
    { "102_qwerty_dot_nodead",	"102 QWERTY (Dot Non-Dead Key)" },
    { "modern", "Modern" },
    { "simple", "Simple" },
    { "polytonic", "Polytonic" },
    { "bds", "BDS" },
    { "bekl", "BEKL" },
    { "bas_phonetic", "Basque (Phonetic)" },
    { "modern-cyr", "Modern (Cyrillic)" },
    { "ykeydvorak",	"Y-key Dvorak" },
    { "minuskeydvorak",	"Minus Key Dvorak" },
    { "dvorakprogr",	"Dvorak Programmer" },
    { "ykeydvorakprogr",	"Y-key Dvorak Programmer" },
    { "minuskeydvorakprogr",	"Minus Key Dvorak Programmer" },
    { "apostrophecolemak",	"Apostrophe Colemak" },
    { "tilde", "With Tilde" },
    { "adapted", "Adapted" },
    { "apostrophe", "With Apostrophe" },
    { "apostrophe-deadquotes", "Apostrophe Dead Quotes" },
    { "colemak", "Colemak" },
    { "colemak_dh", "Colemak (Dvorak Hand)" },
    { "colemak_wide", "Colemak Wide" },
    { "colemak_dh_iso", "Colemak (Dvorak Hand ISO)" },
    { "colemak_dh_wide", "Colemak (Dvorak Hand Wide)" },
    { "colemak_dh_wide_iso", "Colemak (Dvorak Hand Wide ISO)" },
    { "colemak_dh_ortho", "Colemak (Dvorak Hand Ortholinear)" },
    { "altgr-intl", "AltGr International" },
    { "altgr-weur", "AltGr Western European" },
    { "alternatequotes", "Alternate Quotes" },
    { "ibm", "IBM" },
    { "winkeys", "Windows Keys" },
    { "fur", "Friulian" },
    { "lld", "Ladin" },
    { "lldde", "Ladin (Dolomite Edition)" },
    { "unicode", "Unicode" },
    { "unicodeus", "US (Unicode)" },
    { "ike", "Ike" },
    { "multix", "Multilingual Extended" },
    { "olpc", "OLPC" },
    { "olpc2", "OLPC2" },
    { "olpcm", "OLPCM" },
    { "crd", "Cherokee" },
    { "chr", "Cherokee (Alternative)" },
    { "crh", "Crimean Tatar" },
    { "crh_f", "Crimean Tatar (F Style)" },
    { "crh_alt", "Crimean Tatar (Alternative)" },
    { "macOS", "MacOS" },
    { "typewriter", "Typewriter" },
    { "homophonic", "Homophonic" },
    { "ogam", "Ogham" },
    { "tt", "Tatar" },
    { "os_legacy", "OS Legacy" },
    { "os_winkeys", "OS Windows Keys" },
    { "cv", "Chuvash" },
    { "udm", "Udmurt" },
    { "cv_latin", "Chuvash (Latin)" },
    { "kom", "Komi" },
    { "sah", "Sakha" },
    { "xal", "Kalmyk" },
    { "dos", "DOS" },
    { "bak", "Bashkir" },
    { "srp", "Serbian" },
    { "chm", "Mari" },
    { "smi", "Sami" },
    { "smi_nodeadkeys", "Sami (No Dead Keys)" },
    { "phonetic_azerty", "Phonetic (AZERTY)" },
    { "phonetic_dvorak", "Phonetic (Dvorak)" },
    { "ab", "Abkhaz" },
    { "rulemak", "Rulemak" },
    { "chu", "Chuvash (Variant)" },
    { "phonetic_winkeys", "Phonetic (Windows Keys)" },
    { "phonetic_YAZHERTY", "Phonetic (YAZHERTY)" },
    { "gost-6431-75-48", "GOST 6431-75-48" },
    { "gost-14289-88", "GOST 14289-88" },
    { "ruu", "Ruru" },
    { "prxn", "Proxen" },
    { "unipunct", "Unipunct" },
    { "phonetic_mac", "Phonetic Mac" },
    { "ruchey_ru", "Ruchey Russian" },
    { "ruchey_en", "Ruchey English" },
    { "lefty", "Lefty" },
    { "dvorak_quotes", "Dvorak (Quotes)" },
    { "dvorak_altquotes", "Dvorak (Alternative Quotes)" },
    { "szl", "Silesian" },
    { "csb", "Kashubian" },
    { "ru_phonetic_dvorak", "Russian Phonetic (Dvorak)" },
    { "colemak_dh_ansi", "Colemak (Dvorak Hand ANSI)" },
    { "glagolica", "Glagolitic" },
    { "winkeys-p", "Windows Keys (P)" },
    { "diktor", "DikTor" },
    { "ruintl_ru", "Russian International" },
    { "ruintl_en", "Russian International (English)" },
    { "typo", "Typo" },
    { "rtu", "RTU" },
    { "ogam_is434", "Ogham (IS434)" },
    { "def_102",	"Default (102 Keys)" },
    { "def_101",	"Default (101 Keys)" },
    { "def_qwerty",	"Default (QWERTY)" },
    { "def_qwertz",	"Default (QWERTZ)" },
    { "def_dot",	"Default (Dot)" },
    { "def_dead",	"Default (Dead Key)" },
    { "def_nodead",	"Default (Non-Dead Key)" },
    { "def_common",	"Default (Common)" },
    { "oldhunlig",	"Old Hungarian Ligature" },
    { "oldhun_base",	"Old Hungarian Base" },
    { "oldhun_lig",	    "Old Hungarian Ligature (Extended)" },
    { "oldhun_sk_sh",	"Old Hungarian Slovak (Š variant)" },
    { "oldhun_sk_sz",	"Old Hungarian Slovak (Ť variant)" },
    { "SK_Sh",	        "Slovak (Š variant)" },
    { "SK_Sz",	        "Slovak (Ť variant)" },
    { "ohu_sk_fullcaps_req",	"Old Hungarian (Slovak, Full Caps Required)" },
    { "ohu_sk_nocaps_req",	    "Old Hungarian (Slovak, No Caps Required" },
    { "ohu_sk_semicaps_req",	"Old Hungarian (Slovak, Semi-Caps Required" },
    { "oldhun_magic_numpad",	"Old Hungarian Magic Numpad" },
    { "UnicodeExpert", "Unicode (Expert)" },
    { "haw", "Hawaiian" },
    { "swa", "Swahili" },
    { "kik", "Kikuyu" },
    { "std", "Standard" },
    { "us", "United States" },
    { "fr", "French" },
    { "fr-dvorak", "French (Dvorak)" },
    { "fr-legacy", "French (Legacy)" },
    { "eng", "English" },
    { "kut", "Kutchi" },
    { "shs", "Shoshoni" },
    { "intl-unicode", "International Unicode" },
    { "alt-intl-unicode", "Alternative International Unicode" },
    { "hbs", "Hebrew (Standard)" },
    { "workman", "Workman" },
    { "workman-intl", "Workman (International)" },
    { "workman-p", "Workman Programmer" },
    { "carpalx", "Carpalx" },
    { "carpalx-intl", "Carpalx (International)" },
    { "carpalx-altgr-intl", "Carpalx (AltGr International)" },
    { "carpalx-full", "Carpalx (Full)" },
    { "carpalx-full-intl", "Carpalx (Full International)" },
    { "carpalx-full-altgr-intl", "Carpalx (Full AltGr International)" },
    { "cz_sk_de", "Czech/Slovak/German" },
    { "cz_sk_pl_de_es_fi_sv", "Czech/Slovak/Polish/German/Spanish/Finnish/Swedish" },
    { "symbolic", "Symbolic" },
    { "scn", "Sicilian" },
    { "ats", "Azerbaijani (Turkey)" },
    { "common", "Common" },
    { "kana", "Kana" },
    { "kana86", "Kana 86" },
    { "henkan", "Henkan" },
    { "tw", "Taiwan" },
    { "saisiyat", "Saisiyat" },
    { "deva",	        "Devanagari" },
    { "bolnagri",	    "Bolnagri" },
    { "ben",	        "Bengali" },
    { "ben_probhat",	"Bengali (Probhat)" },
    { "ben_baishakhi",	"Bengali (Baishakhi)" },
    { "ben_inscript",	"Bengali (Inscript)" },
    { "ben_gitanjali",	"Bengali (Gitanjali)" },
    { "ben_bornona",	"Bengali (Bornona)" },
    { "ben-kagapa",	    "Bengali (Kagapa)" },
    { "asm-kagapa",	    "Assamese (Kagapa)" },
    { "guj",	        "Gujarati" },
    { "kan",	        "Kannada" },
    { "mal",	        "Malayalam" },
    { "mal_lalitha",	"Malayalam (Lalitha)" },
    { "sat",	        "Santali" },
    { "ori",	        "Oriya" },
    { "ori-bolnagri",	"Oriya (Bolnagri)" },
    { "ori-wx",	        "Oriya (WX)" },
    { "tam",	        "Tamil" },
    { "tam_tamilnumbers",	"Tamil (Tamil Numbers)" },
    { "tamilnet",	        "TamilNet" },
    { "tamilnet_tamilnumbers",	"TamilNet (Tamil Numbers)" },
    { "tamilnet_TSCII",	"TamilNet (TSCII)" },
    { "tamilnet_TAB",	"TamilNet (TAB)" },
    { "tel",	        "Telugu" },
    { "tel-sarala",	    "Telugu (Sarala)" },
    { "urd-phonetic",	"Urdu (Phonetic)" },
    { "urd-phonetic3",	"Urdu (Phonetic 3)" },
    { "urd-winkeys",	"Urdu (Windows Keys)" },
    { "guru",	        "Gurmukhi" },
    { "jhelum",	        "Jhelum" },
    { "hin-wx",	        "Hindi (WX)" },
    { "mal_enhanced",	"Malayalam (Enhanced)" },
    { "mal_poorna",	    "Malayalam (Poorna)" },
    { "hin-kagapa",	    "Hindi (Kagapa)" },
    { "san-kagapa",	    "Sanskrit (Kagapa)" },
    { "mar-kagapa",	    "Marathi (Kagapa)" },
    { "kan-kagapa",	    "Kannada (Kagapa)" },
    { "tel-kagapa",	    "Telugu (Kagapa)" },
    { "guj-kagapa",	    "Gujarati (Kagapa)" },
    { "mni",	        "Manipuri" },
    { "iipa",	        "IIPA (International Phonetic Alphabet)" },
    { "marathi",	    "Marathi" },
    { "san-misc",	    "Sanskrit (Miscellaneous)" },
    { "modi-kagapa",	"Modi (Kagapa)" },
    { "urd-navees",	    "Urdu (Navees)" },
    { "ku",             "Kurdish" },
    { "ku_f",           "Kurdish (F Style)" },
    { "ku_alt",         "Kurdish (Alternative)" },
    { "ku_ara",         "Kurdish (Arabic Script)" },
    { "french",         "French" },
    { "oss",            "Ossian" },
    { "oss_latin9",     "Ossian (Latin9)" },
    { "latin9_nodeadkeys", "Latin9 (No Dead Keys)" },
    { "oss_nodeadkeys", "Ossian (No Dead Keys)" },
    { "bepo", "Bépo" },
    { "bepo_latin9", "Bépo (Latin9)" },
    { "bepo_afnor", "Bépo (AFNOR)" },
    { "ergol", "Ergo-L" },
    { "ergol_iso", "Ergo-L (ISO)" },
    { "bre", "Breton" },
    { "oci", "Occitan" },
    { "geo", "Georgian" },
    { "us-alt", "United States (Alternative)" },
    { "us-azerty", "United States (AZERTY)" },
    { "french", "French" },
    { "afnor", "AFNOR" },
    { "tifinagh", "Tifinagh" },
    { "tifinagh-phonetic", "Tifinagh (Phonetic)" },
    { "tifinagh-extended", "Tifinagh (Extended)" },
    { "tifinagh-extended-phonetic", "Tifinagh (Extended Phonetic)" },
    { "tifinagh-alt", "Tifinagh (Alternative)" },
    { "tifinagh-alt-phonetic", "Tifinagh (Alternative Phonetic)" },
    { "rif", "Rif" },
    { "sun_type6", "Sun Type 6" },
    { "sun_type7", "Sun Type 7" },
    { "sun_type6_suncompat", "Sun Type 6 (Legacy)" },
    { "sun_type7_suncompat", "Sun Type 7 (Legacy)" },
    { "suncompat", "Sun Compatibility" },
    { "nicola_f_bs", "Nicola F Backspace" },
    { "hztg_escape", "HZTG Escape" },
    { "3l", "3-Layer" },
    { "3l-cros", "3-Layer ChromeOS" },
    { "3l-emacs", "3-Layer Emacs" },
    { "drix", "Drix" },
    { "de_se_fi", "German/Swedish/Finnish" },
    { "rus", "Russian" },
    { "ibm238l", "IBM 238L" },
    { "dvp", "DVP" },
    { "norman", "Norman" },
    { "latin", "Latin" },
    { "latin9", "Latin9" },
    { "dvorak", "Dvorak" },
    { "dvorak-intl", "Dvorak (International)" },
    { "dvorak-alt-intl", "Dvorak (Alternative International)" },
    { "dvorak-classic", "Dvorak (Classic)" },
    { "dvorak-mac", "Dvorak (Mac)" },
    { "dvorak-l", "Dvorak (Left-hand)" },
    { "dvorak-r", "Dvorak (Right-hand)" },
    { "nativo", "Nativo" },
    { "nativo-us", "Nativo US" },
    { "nativo-epo", "Nativo Esperanto" },
    { "basic", "Basic" },
    { "cyrillic", "Cyrillic" },
    { "legacy", "Legacy" },
    { "qwerty", "QWERTY" },
    { "qwertz", "QWERTZ" },
    { "azerty", "AZERTY" },
    { "arabic", "Arabic" },
    { "thinkpad", "Thinkpad" },
    { "ergonomic", "Ergonomic" },
    { "ru", "Russian" },
    { "phonetic", "Phonetic" },
    { "biblical", "Biblical" },
    { "indigenous", "Indigenous" }
};

QString getLayoutName(const QString &layoutCode)
{
    return layoutMap.value(layoutCode, layoutCode);
}

QString getLayoutCode(const QString &layoutName)
{
    return layoutMap.key(layoutName, layoutName);
}

QString getVariantName(const QString &variantCode)
{
    return variantMap.value(variantCode, variantCode);
}

QString getVariantCode(const QString &variantName)
{
    return variantMap.key(variantName, variantName);
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

QString getCurrentKeymapLayout() {
    std::array<char, 128> buffer;
    auto closer = [](FILE* f) { return pclose(f); };
    std::unique_ptr<FILE, decltype(closer)> pipe(popen("setxkbmap -query", "r"), closer);

    if (!pipe) {
        std::cerr << "Failed to execute command!" << std::endl;
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());

        if (line.find("layout:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key, layout;
            iss >> key >> layout;
            return QString::fromStdString(layout);
        }
    }

    return "";
}
