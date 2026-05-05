// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FEATURE_H
#define FEATURE_H

#include <string>
#include <vector>
#include <utility>
#include <sensors/sensors.h>

/**
 * @brief Feature class is providing RAII-style for lm_sensors library
 */

class Feature
{
public:
    Feature(const sensors_chip_name*, const sensors_feature*);
    const std::string& getName() const;
    const std::string& getLabel() const;
    double getValue(sensors_subfeature_type) const;
    sensors_feature_type getType() const;
private:
    // Do not try to change these chip names, as they point to internal structures of lm_sensors!
    const sensors_chip_name* mSensorsChipName;

    const sensors_feature* mSensorsFeature;

    // "Printable" feature label
    std::string mLabel;

    std::vector<const sensors_subfeature*> mSubFeatures;
};

#endif // CHIP_H
