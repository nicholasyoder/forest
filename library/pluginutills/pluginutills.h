// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINUTILLS_H
#define PLUGINUTILLS_H


#include <QStringList>

class pluginutills
{
public:
    pluginutills();

    // Enabled app plugins from Forest.conf [plugins]
    static QStringList get_plugin_paths();

};

#endif // PLUGINUTILLS_H
