// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SENSORS_H
#define SENSORS_H

#include "chip.h"
#include <vector>
#include <sensors/sensors.h>

/**
 * @brief Sensors class is providing RAII-style for lm_sensors library
 */

class Sensors
{
public:
    Sensors();
    ~Sensors();
    const std::vector<Chip>& getDetectedChips() const;

private:
    static std::vector<Chip> mDetectedChips;

    /**
     * lm_sensors library can be initialized only once so this will tell us when to init
     * and when to clean up.
     */
    static int mInstanceCounter;
    static bool mSensorsInitialized;
};

#endif // SENSORS_H
