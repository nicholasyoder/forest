// SPDX-License-Identifier: LGPL-3.0-or-later

#include "chip.h"
#include <QtCore/QDebug>

Chip::Chip(const sensors_chip_name* sensorsChipName)
    : mSensorsChipName(sensorsChipName)
{
    const int BUF_SIZE = 256;
    char buf[BUF_SIZE];
    if (sensors_snprintf_chip_name(buf, BUF_SIZE, mSensorsChipName) > 0)
    {
        mName = std::string(buf);
    }

    qDebug() << "Detected chip:" << QString::fromStdString(mName);

    const sensors_feature* feature;
    int featureNr = 0;

    while ((feature = sensors_get_features(mSensorsChipName, &featureNr)))
    {
        mFeatures.push_back(Feature(mSensorsChipName, feature));
    }
}

const std::string& Chip::getName() const
{
    return mName;
}

const std::vector<Feature>& Chip::getFeatures() const
{
    return mFeatures;
}
