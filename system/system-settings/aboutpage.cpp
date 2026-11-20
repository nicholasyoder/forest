// SPDX-License-Identifier: LGPL-3.0-or-later

#include "aboutpage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QFile>
#include <QLocale>
#include <QPixmap>
#include <QSysInfo>

AboutPage::AboutPage(){
    settings_item = new settings_page("about/about", "About", "help-about");
    settings_item->set_keywords({"system", "info", "version", "hardware", "computer", "ram"});
    connect(settings_item, &settings_category::opened, this, &AboutPage::load_about_data);

    QFrame *logo_base = new QFrame;
    QHBoxLayout *logo_h_layout = new QHBoxLayout(logo_base);
    logo_h_layout->setContentsMargins(QMargins(10,10,10,10));
    logo_h_layout->setSpacing(0);
    logo_h_layout->addStretch(1);
    QVBoxLayout *logo_v_layout = new QVBoxLayout;
    logo_v_layout->setContentsMargins(QMargins(0,0,0,0));
    logo_v_layout->setSpacing(5);
    logo_image = new QLabel;
    QHBoxLayout *logo_center_layout = new QHBoxLayout;
    logo_center_layout->setContentsMargins(QMargins(0,0,0,0));
    logo_center_layout->setSpacing(0);
    logo_center_layout->addStretch(1);
    logo_center_layout->addWidget(logo_image);
    logo_center_layout->addStretch(1);
    logo_v_layout->addLayout(logo_center_layout);
    logo_label = new QLabel("Forest 0.0.0");
    QFont f;
    f.setPointSize(26);
    logo_label->setFont(f);
    logo_v_layout->addWidget(logo_label);
    logo_h_layout->addLayout(logo_v_layout);
    logo_h_layout->addStretch(1);
    settings_widget *logo_item = new settings_widget("", "", logo_base, true);
    settings_item->add_child(logo_item);

    settings_widget_group *info_wg = new settings_widget_group;
    settings_item->add_child(info_wg);

    os_name_label = new QLabel;
    os_name_label->setObjectName("SystemInfoLabel");
    settings_widget *os_name_item = new settings_widget("OS Name", "", os_name_label);
    info_wg->add_child(os_name_item);
    kernel_version_label = new QLabel;
    kernel_version_label->setObjectName("SystemInfoLabel");
    settings_widget *kernel_version_item = new settings_widget("Kernel Version", "", kernel_version_label);
    info_wg->add_child(kernel_version_item);
    cpu_model_label = new QLabel;
    cpu_model_label->setObjectName("SystemInfoLabel");
    settings_widget *cpu_model_item = new settings_widget("CPU Model", "", cpu_model_label);
    info_wg->add_child(cpu_model_item);
    architecture_label = new QLabel;
    architecture_label->setObjectName("SystemInfoLabel");
    settings_widget *architecture_item = new settings_widget("Architecture", "", architecture_label);
    info_wg->add_child(architecture_item);
    memory_label = new QLabel;
    memory_label->setObjectName("SystemInfoLabel");
    settings_widget *memory_item = new settings_widget("Usable Memory", "", memory_label);
    info_wg->add_child(memory_item);
    hostname_label = new QLabel;
    hostname_label->setObjectName("SystemInfoLabel");
    settings_widget *hostname_item = new settings_widget("Host Name", "", hostname_label);
    info_wg->add_child(hostname_item);
}

void AboutPage::load_about_data(){
    logo_label->setText("Forest " FOREST_VERSION);
    logo_image->setPixmap(QPixmap("/usr/share/forest/pics/logo.png"));

    os_name_label->setText(QSysInfo::prettyProductName());
    kernel_version_label->setText(QSysInfo::kernelVersion());
    architecture_label->setText(QSysInfo::currentCpuArchitecture());
    hostname_label->setText(QSysInfo::machineHostName());
    cpu_model_label->setText(proc_value("/proc/cpuinfo", "model name"));

    qint64 mem_kb = proc_value("/proc/meminfo", "MemTotal").split(' ', Qt::SkipEmptyParts).value(0).toLongLong();
    memory_label->setText(mem_kb > 0 ? QLocale().formattedDataSize(mem_kb * 1024, 1) : QString());
}

// First "<key>: value" line of a /proc file.
QString AboutPage::proc_value(const QString &path, const QString &key){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    // /proc files report size 0, so atEnd()/readLine() are unreliable; read it whole.
    const QList<QByteArray> lines = file.readAll().split('\n');
    for (const QByteArray &raw : lines) {
        const QString line = QString::fromUtf8(raw);
        if (!line.startsWith(key)) continue;
        int colon = line.indexOf(':');
        if (colon >= 0) return line.mid(colon + 1).trimmed();
    }
    return QString();
}
