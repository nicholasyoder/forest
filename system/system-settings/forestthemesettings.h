#ifndef FORESTTHEMESETTINGS_H
#define FORESTTHEMESETTINGS_H

#include <QObject>

#include "../../settings/widgets/listwidget.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class ForestThemeSettings : public QObject
{
    Q_OBJECT
public:
    ForestThemeSettings();

public slots:
    settings_category* get_settings_item(){ return settings_item; }

signals:

private slots:
    void load_desktop_themes();
    void set_desktop_theme(QListWidgetItem *theme_item);

private:
    settings_category *settings_item = nullptr;
    ListWidget *forest_theme_list = nullptr;

};

#endif // FORESTTHEMESETTINGS_H
