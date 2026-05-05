// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHIP_H
#define CHIP_H

#include "feature.h"
#include <string>
#include <vector>
#include <utility>
#include <sensors/sensors.h>

/**
 * @brief Chip class is providing RAII-style for lm_sensors library
 */

class Chip
{
public:
    Chip(const sensors_chip_name*);
    const std::string& getName() const;
    const std::vector<Feature>& getFeatures() const;

private:
    // Do not try to change these chip names, as they point to internal structures of lm_sensors!
    const sensors_chip_name* mSensorsChipName;

    // "Printable" chip name
    std::string mName;

    std::vector<Feature> mFeatures;
};

#endif // CHIP_H
