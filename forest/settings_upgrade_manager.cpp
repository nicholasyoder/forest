#include "settings_upgrade_manager.h"

#include <QDir>
#include <QProcess>
#include <QDebug>

SettingsUpgradeManager::SettingsUpgradeManager(QObject *parent) : QObject{parent}{
    settings = new QSettings("Forest","Forest");
}

void SettingsUpgradeManager::perform_upgrades(){
    QString version = settings->value("version").toString();
    QSettings default_settings("/etc/forest/Forest.conf", QSettings::IniFormat);
    QString current_version = default_settings.value("version").toString();

    // Load defaults and return if unable to find existing settings
    if(version == "") {
        load_defaults();
        return;
    }

    // Skip upgrades if already up to date
    if(version == current_version) return;

    // Register upgrade functions here
    QMap<QVector<int>, std::function<void()>> upgrade_map = {
        {{0,7,9}, [this](){ this->upgrade_0_7_9(); }},
    };

    // Get version of existing settings
    QVector<int> existing_version;
    foreach (QString part, settings->value("version").toString().split('.'))
        existing_version.append(part.toInt());

    // Run upgrades from that version until latest
    for (auto it = upgrade_map.begin(); it != upgrade_map.end(); ++it)
        if (it.key() > existing_version) {
            QStringList v_parts;
            foreach(int part, it.key()) v_parts << QString::number(part);
            qDebug() << "Upgrading settings to version " + v_parts.join(".");
            it.value()(); // Call the upgrade function
        }

    // Set version to latest
    settings->setValue("version", current_version);
}

void SettingsUpgradeManager::load_defaults(){
    qDebug() << "Loading default settings...";

    QString defaults_dir_path = "/etc/forest";
    QDir defaults_dir(defaults_dir_path);
    if (!defaults_dir.exists()) {
        qDebug() << "Failed to load default settings. " << defaults_dir_path << " does not exist.";
        return;
    }

    QString dest_dir_path = QDir::homePath() + "/.config/Forest";
    QDir dest_dir(dest_dir_path);
    if (dest_dir.exists()){
        QString bkup_path = dest_dir_path + "_backup";
        qDebug() << "Found existing settings. Moving them to " << bkup_path;
        dest_dir.rename(dest_dir_path, bkup_path);
    }
    qDebug() << "Creating settings directory: " << dest_dir_path;
    dest_dir.mkdir(dest_dir_path);

    foreach(QString conf_file, defaults_dir.entryList(QDir::Files)){
        QString src_path = defaults_dir_path + "/" + conf_file;
        QString dest_path = dest_dir_path + "/" + conf_file;
        qDebug() << "Copying " << src_path << "to" << dest_path;
        if(!QFile::copy(src_path, dest_path))
            qDebug() << "Failed to copy " << src_path << "to" << dest_path;
    }
}

void SettingsUpgradeManager::upgrade_0_7_9(){
    settings->remove("needDefaults"); // Using version key now
}
