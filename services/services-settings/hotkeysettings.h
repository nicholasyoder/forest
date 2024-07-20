#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H

#include <QObject>

#include "../../library/pluginutills/settings_plugin_interface.h"

class HotkeySettings : public QObject
{
    Q_OBJECT
public:
    HotkeySettings();

public slots:
    settings_category* get_settings_item(){ return settings_item; }

signals:

private slots:
    void load_hotkeys();

private:
    settings_category *settings_item = nullptr;

};

#endif // HOTKEYSETTINGS_H
