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

#include <QList>

static const QList<QPair<QString, QString>> timezoneList {
    { "GMT-12",	"Baker Island, Howland Island (uninhabited)" },
    { "GMT-11",	"American Samoa, Niue, Midway Atoll" },
    { "GMT-10",	"Hawaii (USA), Cook Islands, French Polynesia (Tahiti)" },
    { "GMT-9",  "Alaska (USA), Gambier Islands (French Polynesia)" },
    { "GMT-8",  "Pacific Time (USA, Canada), Clipperton Island, Pitcairn Islands" },
    { "GMT-7",  "Mountain Time (USA, Canada), Sonora (Mexico)" },
    { "GMT-6",  "Central Time (USA, Canada), Costa Rica, El Salvador, Guatemala, Honduras, Nicaragua" },
    { "GMT-5",  "Eastern Time (USA, Canada), Cuba, Colombia, Ecuador (mainland), Peru" },
    { "GMT-4",  "Atlantic Time (Canada), Bolivia, Brazil (Amazonas), Chile, Paraguay, Venezuela" },
    { "GMT-3",  "Argentina, Brazil (Brasília, São Paulo), Falkland Islands, Uruguay" },
    { "GMT-2",  "Fernando de Noronha (Brazil), South Georgia and the South Sandwich Islands" },
    { "GMT-1",  "Azores (Portugal), Cape " },
    { "GMT-0",  "Iceland, United Kingdom (winter), Portugal (mainland, winter), Ghana, Senegal" },
    { "GMT+1",  "Central European Time (CET): Germany, France, Spain, Italy, Norway, Poland" },
    { "GMT+2",  "Eastern European Time (EET): Egypt, Greece, South Africa, Ukraine" },
    { "GMT+3",  "Moscow Time (Russia), Turkey, Saudi Arabia, Kenya, Ethiopia" },
    { "GMT+4",  "United Arab Emirates, Oman, Azerbaijan, Georgia, Réunion (France)" },
    { "GMT+5",  "Pakistan, Kazakhstan (western regions), Maldives, Uzbekistan" },
    { "GMT+6",  "Bangladesh, Bhutan, Kazakhstan (eastern regions), Kyrgyzstan" },
    { "GMT+7",  "Thailand, Vietnam, Cambodia, Indonesia (western regions), Laos" },
    { "GMT+8",  "China, Philippines, Malaysia, Singapore, Western Australia, Taiwan" },
    { "GMT+9",  "Japan, South Korea, North Korea, Eastern Indonesia, Palau" },
    { "GMT+10",	"Eastern Australia, Papua New Guinea, Guam, Northern Mariana Islands" },
    { "GMT+11",	"Solomon Islands, Vanuatu, New Caledonia (France), Norfolk Island" },
    { "GMT+12",	"New Zealand, Fiji, Marshall Islands, Tuvalu, Wallis and Futuna" },
    { "GMT+13",	"Tonga, Samoa, Tokelau" },
    { "GMT+14",	"Line Islands (Kiribati), Samoa (during daylight saving time)" },
};
