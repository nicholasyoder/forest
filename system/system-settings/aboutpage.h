#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QObject>
#include <QLabel>
#include <QDebug>

#include "../../library/pluginutills/settings_plugin_interface.h"

class AboutPage : public QObject
{
    Q_OBJECT
public:
    AboutPage();

public slots:
    settings_category* get_settings_item(){ return settings_item; }

signals:

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

    settings_category *settings_item = nullptr;
};

#endif // ABOUTPAGE_H
