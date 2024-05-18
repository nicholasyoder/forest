#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>

#include "../../library/pluginutills/settings_plugin_interface.h"
#include "miscutills.h"

class SystemSettings : public QObject, settings_plugin_infterace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.settings.system.plugin")
    Q_INTERFACES(settings_plugin_infterace)
public:
    SystemSettings();

    QList<settings_item*> get_settings_items();

private slots:
    void load_about_data();

private:
    QLabel *logo_image = nullptr;
    QLabel *os_name_label = nullptr;
    QLabel *kernel_version_label = nullptr;
    QLabel *cpu_model_label = nullptr;
    QLabel *architecture_label = nullptr;
    QLabel *memory_label = nullptr;
    QLabel *hostname_label = nullptr;
};

#endif // SYSTEMSETTINGS_H
