// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sensors.h"
#include <QtCore/QDebug>

std::vector<Chip> Sensors::mDetectedChips = std::vector<Chip>();
int Sensors::mInstanceCounter = 0;
bool Sensors::mSensorsInitialized = false;

Sensors::Sensors()
{
    // Increase instance counter
    ++mInstanceCounter;

    if (!mSensorsInitialized && sensors_init(NULL) == 0)
    {
        // Sensors initialized
        mSensorsInitialized = true;

        sensors_chip_name const * chipName;
        int chipNr = 0;
        while ((chipName = sensors_get_detected_chips(NULL, &chipNr)) != NULL)
        {
            mDetectedChips.push_back(chipName);
        }

        qDebug() << "lm_sensors library initialized";
    }
}

Sensors::~Sensors()
{
    // Decrease instance counter
    --mInstanceCounter;

    if (mInstanceCounter == 0 && mSensorsInitialized)
    {
        mDetectedChips.clear();
        mSensorsInitialized = false;
        sensors_cleanup();

        qDebug() << "lm_sensors library cleanup";
    }
}

const std::vector<Chip>& Sensors::getDetectedChips() const
{
    return mDetectedChips;
}

