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
    QDir dir("/etc/forest");
    if (dir.exists()) {
        QStringList dirs = dir.entryList();

        QDir destdir(QDir::homePath() + "/.config/Forest");
        if (!destdir.exists())
            destdir.mkdir(destdir.absolutePath());

        int c = 2;
        while (c < dirs.length()) {
            QProcess cp;
            cp.start("/usr/bin/cp", QStringList() << dir.absolutePath() + dirs.at(c) << destdir.absolutePath() + dirs.at(c));
            cp.waitForFinished();
            c++;
        }
    }
    else {
        qDebug() << "Failed to load default settings. " + dir.absolutePath() + " does not exist.";
    }
}

void SettingsUpgradeManager::upgrade_0_7_9(){
    settings->remove("needDefaults"); // Using version key now
}
