// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SENSORSCONFIG_H
#define SENSORSCONFIG_H

#include <QColor>
#include <QString>

#include "sensor/chip.h"

// QSettings("Forest", "Temperature Monitor") keys and defaults, shared by the applet and its settings page.
// Temperatures are stored in Celsius whatever the scale.
namespace sensorsconfig {

constexpr char path[] = "desktop/panel/sensors"; // must match sensors.json's "settings"
constexpr char updateinterval[] = "TimeUpdat"; // seconds
constexpr int updateinterval_default = 3;
constexpr char fahrenheit[] = "Fahrenheit";
constexpr bool fahrenheit_default = false;
constexpr char display[] = "display";
constexpr char display_text[] = "text";
constexpr char display_bars[] = "bars";
constexpr char sensor[] = "sensor"; // sensor_id() shown in text mode; first sensor if unset
constexpr char warningtemp[] = "warningtemp";
constexpr int warningtemp_default = 70;
constexpr char criticaltemp[] = "criticaltemp";
constexpr int criticaltemp_default = 90;
constexpr char hiddenbars[] = "hiddenbars"; // sensor_id()s left out of bars mode
constexpr char barwidth[] = "barwidth";
constexpr int barwidth_default = 5;
constexpr char barspacing[] = "barspacing";
constexpr int barspacing_default = 2;
constexpr char margin[] = "margin";
constexpr int margin_default = 2;
constexpr char maxtemp[] = "maxtemp";
constexpr int maxtemp_default = 110;
constexpr char backcolor[] = "backcolor";
inline const QColor backcolor_default = Qt::black;

// Labels repeat across chips (temp1, ...), so the chip name is part of the ID.
inline QString sensor_id(const Chip &chip, const Feature &feature){
    return QString::fromStdString(chip.getName() + "/" + feature.getLabel());
}

}

#endif // SENSORSCONFIG_H
