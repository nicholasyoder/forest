/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Łukasz Twarduś <ltwardus@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

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
