// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CURSORTHEMESETTINGS_H
#define CURSORTHEMESETTINGS_H

#include <QObject>
#include <QDebug>
#include <QSpinBox>

#include "listwidget.h"
#include "wideicondelegate.h"
#include "../../library/pluginutills/settings_plugin_interface.h"

class CursorThemeSettings : public QObject
{
    Q_OBJECT
public:
    CursorThemeSettings();

public slots:
    settings_page* get_settings_item(){ return settings_item; }
    void set_cursor_theme(QListWidgetItem *item);

signals:

private slots:
    void load_cursor_themes();
    QPixmap get_cursor_preview(QString theme, QString cursor, int size = 32);
    QPixmap combine_pixmaps(QList<QPixmap> pixmaps);
    void set_cursor_size();

private:
    settings_page *settings_item = nullptr;
    ListWidget *cursor_theme_list = nullptr;
    QSpinBox *size_input = nullptr;
};

#endif // CURSORTHEMESETTINGS_H
