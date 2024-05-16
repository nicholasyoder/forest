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
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

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
