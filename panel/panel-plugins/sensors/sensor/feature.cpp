// SPDX-License-Identifier: LGPL-3.0-or-later

#include "feature.h"
#include <QtCore/QDebug>
#include <cstdlib>

Feature::Feature(const sensors_chip_name* sensorsChipName, const sensors_feature* sensorsFeature)
    : mSensorsChipName(sensorsChipName),
      mSensorsFeature(sensorsFeature)
{
    char *featureLabel = NULL;

    if ((featureLabel = sensors_get_label(mSensorsChipName, mSensorsFeature)))
    {
        mLabel = featureLabel;
        free(featureLabel);
    }

    qDebug() << "Detected feature:" << QString::fromStdString(std::string(sensorsFeature->name))
             << "(" << QString::fromStdString(mLabel) << ")";
}

const std::string& Feature::getLabel() const
{
    return mLabel;
}

double Feature::getValue(sensors_subfeature_type subfeature_type) const
{
    double result = 0;

    const sensors_subfeature *subfeature;

    // Find feature
    subfeature = sensors_get_subfeature(mSensorsChipName, mSensorsFeature, subfeature_type);

    if (subfeature)
    {
        sensors_get_value(mSensorsChipName, subfeature->number, &result);
    }

    return result;
}

sensors_feature_type Feature::getType() const
{
    return mSensorsFeature->type;
}
