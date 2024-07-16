#include "systemsettings.h"

#include <QProcess>
#include <QDir>
#include <QSettings>
#include <QDebug>

SystemSettings::SystemSettings()
{
}


QList<settings_item*> SystemSettings::get_settings_items(){
    QList<settings_item*> items;

    settings_category *about_cat = new settings_category("About", "", "help-about");
    connect(about_cat, &settings_category::opened, this, &SystemSettings::load_about_data);
    items.append(about_cat);

    QFrame *logo_base = new QFrame;
    QHBoxLayout *logo_h_layout = new QHBoxLayout(logo_base);
    logo_h_layout->setMargin(10);
    logo_h_layout->setSpacing(0);
    logo_h_layout->addStretch(1);
    QVBoxLayout *logo_v_layout = new QVBoxLayout;
    logo_v_layout->setMargin(0);
    logo_v_layout->setSpacing(5);
    logo_image = new QLabel;
    QHBoxLayout *logo_center_layout = new QHBoxLayout;
    logo_center_layout->setMargin(0);
    logo_center_layout->setSpacing(0);
    logo_center_layout->addStretch(1);
    logo_center_layout->addWidget(logo_image);
    logo_center_layout->addStretch(1);
    logo_v_layout->addLayout(logo_center_layout);
    QLabel *logo_label = new QLabel("Forest 0.8");
    QFont f;
    f.setPointSize(26);
    logo_label->setFont(f);
    logo_v_layout->addWidget(logo_label);
    logo_h_layout->addLayout(logo_v_layout);
    logo_h_layout->addStretch(1);
    settings_widget *logo_item = new settings_widget("", "", logo_base, true);
    about_cat->add_child(logo_item);

    settings_widget_group *info_wg = new settings_widget_group;
    about_cat->add_child(info_wg);

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
    settings_widget *memory_item = new settings_widget("Memory", "", memory_label);
    info_wg->add_child(memory_item);
    hostname_label = new QLabel;
    hostname_label->setObjectName("SystemInfoLabel");
    settings_widget *hostname_item = new settings_widget("Host Name", "", hostname_label);
    info_wg->add_child(hostname_item);


    settings_category *themes_cat = new settings_category("Themes", "", "preferences-desktop-theme");
    items.append(themes_cat);

    settings_category *desktop_cat = new settings_category("Desktop", "", "preferences-desktop-color");
    connect(desktop_cat, &settings_category::opened, this, &SystemSettings::load_desktop_themes);
    themes_cat->add_child(desktop_cat);
    desktop_theme_list = new ListWidget();
    desktop_theme_list->setMinimumHeight(5);
    desktop_theme_list->setExclusiveCheck(true);
    desktop_theme_list->setSelectionMode(QAbstractItemView::NoSelection);
    settings_widget *theme_list_item = new settings_widget("","", desktop_theme_list);
    desktop_cat->add_child(theme_list_item);

    settings_category *widget_cat = new settings_category("Widget", "", "preferences-desktop-theme");
    themes_cat->add_child(widget_cat);

    settings_category *icon_cat = new settings_category("Icon", "", "preferences-desktop-icons");
    themes_cat->add_child(icon_cat);

    settings_category *cursor_cat = new settings_category("Mouse Cursor", "", "preferences-desktop-mouse");
    themes_cat->add_child(cursor_cat);

    connect(desktop_theme_list, &ListWidget::itemExclusivlySelected, this, &SystemSettings::set_desktop_theme);

    //settings_category *system_cat = new settings_category("System", "", "preferences-system");
    //items.append(system_cat);

    return items;
}

void SystemSettings::load_about_data(){
    logo_image->setPixmap(QPixmap("/usr/share/forest/pics/logo.png"));

    QString os_name = miscutills::run_shell_command("cat /etc/os-release | grep PRETTY_NAME");
    os_name = os_name.remove("PRETTY_NAME=\"");
    os_name = os_name.remove("\"");
    os_name = os_name.remove("\n");
    os_name_label->setText(os_name);

    QString kernel_version = miscutills::run_shell_command("uname -r");
    kernel_version = kernel_version.remove("\n");
    kernel_version_label->setText(kernel_version);

    QString cpu_model = miscutills::run_shell_command("cpumodel=`lscpu | grep 'Model name:'` && echo ${cpumodel:12}");
    cpu_model = cpu_model.remove("\n");
    cpu_model_label->setText(cpu_model);

    QString architecture = miscutills::run_shell_command("cpuarch=`lscpu | grep 'Architecture:'` && echo ${cpuarch:13}");
    architecture = architecture.remove("\n");
    architecture_label->setText(architecture);

    QString memory = miscutills::run_shell_command("memamount=`lsmem | grep 'Total online memory:'` && echo -n ${memamount:21} && echo B");
    memory = memory.remove("\n");
    memory_label->setText(memory);

    QString hostname = miscutills::run_shell_command("hostname");
    hostname = hostname.remove("\n");
    hostname_label->setText(hostname);
}

void SystemSettings::load_desktop_themes(){
    desktop_theme_list->clear();

    QSettings settings("Forest", "Forest");
    QString current_theme = settings.value("theme").toString();

    QDir theme_dir("/usr/share/forest/themes");
    QStringList entrylist = theme_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString entry, entrylist) {
        QListWidgetItem *item = new QListWidgetItem(entry);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry == current_theme ? Qt::Checked : Qt::Unchecked);
        desktop_theme_list->addItem(item);
    }
}

void SystemSettings::set_desktop_theme(QListWidgetItem *theme_item){
    QString new_theme = theme_item->text();
    QSettings settings("Forest", "Forest");
    settings.setValue("theme", new_theme);
    settings.sync();
    miscutills::call_dbus("forest/loadstylesheet");
}
