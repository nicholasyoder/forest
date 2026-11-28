// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forestplatformtheme.h"

#include <qpa/qplatformthemeplugin.h>

class ForestPlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "forestplatformtheme.json")

public:
    QPlatformTheme *create(const QString &key, const QStringList &params) override
    {
        Q_UNUSED(params);
        if (key.compare(QLatin1String("forest"), Qt::CaseInsensitive) == 0)
            return new ForestPlatformTheme;
        return nullptr;
    }
};

#include "main.moc"
