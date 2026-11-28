// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global_settings.h"
#include "../desktopconfig.h"

#include <QStringList>

// Initialization / Defaults
wallpaper::Config GS::WALLPAPER_CONFIG;
bool GS::SHOW_ICONS = desktopconfig::showicons_default;
int GS::ICON_SIZE = desktopconfig::iconsize_default;
int GS::GRID_SPACING = desktopconfig::gridspacing_default;
bool GS::AUTO_ARRANGE = desktopconfig::autoarrange_default;
QString GS::SORT_BY = desktopconfig::sortby_default;
QString GS::SORT_ORDER = desktopconfig::sortorder_default;
bool GS::FOLDERS_FIRST = desktopconfig::foldersfirst_default;
bool GS::SHOW_HIDDEN = desktopconfig::showhidden_default;
bool GS::SINGLE_CLICK = desktopconfig::singleclick_default;

void GS::load(){
    QSettings settings("Forest", "Forest");
    settings.sync();
    settings.beginGroup(desktopconfig::group);

    WALLPAPER_CONFIG = desktopconfig::readWallpaper(settings);
    SHOW_ICONS = settings.value(desktopconfig::showicons, desktopconfig::showicons_default).toBool();
    ICON_SIZE = qBound(desktopconfig::iconsize_min,
                       settings.value(desktopconfig::iconsize, desktopconfig::iconsize_default).toInt(),
                       desktopconfig::iconsize_max);
    GRID_SPACING = qBound(desktopconfig::gridspacing_min,
                          settings.value(desktopconfig::gridspacing, desktopconfig::gridspacing_default).toInt(),
                          desktopconfig::gridspacing_max);
    AUTO_ARRANGE = settings.value(desktopconfig::autoarrange, desktopconfig::autoarrange_default).toBool();
    const QString sortby = settings.value(desktopconfig::sortby, desktopconfig::sortby_default).toString();
    SORT_BY = QStringList{"name", "type", "size", "modified"}.contains(sortby) ? sortby : QString(desktopconfig::sortby_default);
    const QString sortorder = settings.value(desktopconfig::sortorder, desktopconfig::sortorder_default).toString();
    SORT_ORDER = (sortorder == "descending") ? QStringLiteral("descending") : QStringLiteral("ascending");
    FOLDERS_FIRST = settings.value(desktopconfig::foldersfirst, desktopconfig::foldersfirst_default).toBool();
    SHOW_HIDDEN = settings.value(desktopconfig::showhidden, desktopconfig::showhidden_default).toBool();
    SINGLE_CLICK = settings.value(desktopconfig::singleclick, desktopconfig::singleclick_default).toBool();
}
