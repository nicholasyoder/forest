// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings_upgrade_manager.h"

#include "hotkeyconfig.h"

#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QDebug>

// Next free "<prefix>NNNN" key in group, or "" if some entry already has key == value.
static QString next_free_key(QSettings *settings, const QString &group, const QString &prefix,
                             const QString &key, const QString &value){
    settings->beginGroup(group);
    int last = -1;
    bool exists = false;
    foreach (QString child, settings->childGroups()){
        if (settings->value(child + "/" + key).toString() == value) exists = true;
        if (child.startsWith(prefix)) last = qMax(last, child.mid(prefix.length()).toInt());
    }
    settings->endGroup();
    if (exists) return "";
    return group + "/" + prefix + QString("%1").arg(last + 1, 4, 10, QChar('0'));
}

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
        {{0,9,0}, [this](){ this->upgrade_0_9_0(); }},
        {{0,10,0}, [this](){ this->upgrade_0_10_0(); }},
        {{0,10,1}, [this](){ this->upgrade_0_10_1(); }},
        {{0,11,1}, [this](){ this->upgrade_0_11_1(); }},
        {{0,12,0}, [this](){ this->upgrade_0_12_0(); }},
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

    // Only fill in what's missing: a config without `version` may still hold the user's settings.
    QString dest_dir_path = QDir::homePath() + "/.config/Forest";
    QDir().mkpath(dest_dir_path);

    foreach(QString conf_file, defaults_dir.entryList({"*.conf"}, QDir::Files)){
        QString src_path = defaults_dir_path + "/" + conf_file;
        QString dest_path = dest_dir_path + "/" + conf_file;
        if(QFile::exists(dest_path)) continue;
        qDebug() << "Copying " << src_path << "to" << dest_path;
        if(!QFile::copy(src_path, dest_path))
            qDebug() << "Failed to copy " << src_path << "to" << dest_path;
    }
}

void SettingsUpgradeManager::upgrade_0_7_9(){
    settings->remove("needDefaults"); // Using version key now
}

void SettingsUpgradeManager::upgrade_0_9_0(){
    QString hotkey = next_free_key(settings, "hotkeys", "item-", "keysequence", "Meta+L");
    if (!hotkey.isEmpty()){
        settings->setValue(hotkey + "/action", "loginctl lock-session");
        settings->setValue(hotkey + "/description", "Lock screen");
        settings->setValue(hotkey + "/keysequence", "Meta+L");
    }
    // gnome-screenshot can't capture under Biome, and show desktop moved off
    // /org/forest/hotkeys; customized actions are left alone.
    settings->beginGroup("hotkeys");
    foreach (QString child, settings->childGroups()){
        QString action = settings->value(child + "/action").toString();
        if (action == "gnome-screenshot")
            settings->setValue(child + "/action", "forest-screenshot");
        else if (action == "gnome-screenshot -w"){
            settings->setValue(child + "/action", "forest-screenshot --region");
            settings->setValue(child + "/description", "screenshot region");
        }
        else if (action == "DBUS:bus=Session,service=org.forest,path=/org/forest/hotkeys,method=showdesktop")
            settings->setValue(child + "/action", "DBUS:bus=Session,service=org.forest,path=/org/forest/showdesktop,method=toggle");
    }
    settings->endGroup();

    QSettings panel_settings("Forest", "Panel");
    panel_settings.beginGroup("plugins");
    foreach (QString child, panel_settings.childGroups())
        if (panel_settings.value(child + "/path").toString() == "seperator")
            panel_settings.setValue(child + "/path", "separator");
    panel_settings.endGroup();
}

void SettingsUpgradeManager::upgrade_0_11_1(){
    // Wallpaper mode is stored by name now; the old ints were 0 Fill, 1 Fit, 2 Stretch, 3 Tile (dropped), 4 Center.
    const QStringList legacy_modes{"Fill", "Fit", "Stretch", "Fill", "Center"};
    bool is_int = false;
    const int mode = settings->value("desktop/imagemode").toString().toInt(&is_int);
    if (is_int)
        settings->setValue("desktop/imagemode", legacy_modes.value(mode, "Fill"));

    // Hotkeys store a shortcuts-spec `trigger` now, not a Qt `keysequence`.
    settings->beginGroup("hotkeys");
    foreach (QString child, settings->childGroups()){
        const QString legacy = settings->value(child + "/keysequence").toString();
        if (legacy.isEmpty() && !settings->contains(child + "/keysequence")) continue;
        const QString trigger = hotkeyconfig::triggerFromLegacy(legacy);
        if (trigger.isEmpty())
            qWarning() << "Hotkey" << child << "has an unconvertible keysequence" << legacy << "- left unbound";
        else if (!settings->contains(child + "/trigger"))
            settings->setValue(child + "/trigger", trigger);
        settings->remove(child + "/keysequence");
    }
    settings->endGroup();
}

void SettingsUpgradeManager::upgrade_0_12_0(){
    // Desktop icon positions are keyed by filename now, not absolute path (nested QSettings groups).
    const QString prefix = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).mid(1) + "/";
    settings->beginGroup("desktopicons");
    foreach (QString key, settings->allKeys()){
        if (!key.contains('/')) continue;
        const QString name = key.mid(prefix.size());
        if (key.startsWith(prefix) && !name.contains('/') && !settings->contains(name))
            settings->setValue(name, settings->value(key));
        settings->remove(key);
    }
    settings->endGroup();

    // Old defaults shipped iconsize=32 and margins/bottom=26 while icons drew at 48 px; drop both.
    settings->remove("desktop/margins");
    if (settings->value("desktop/iconsize").toInt() == 32)
        settings->remove("desktop/iconsize");
}

void SettingsUpgradeManager::upgrade_0_10_1(){
    // Plugin dir moved from /usr/lib/forest to the build's libdir (/usr/lib64/forest on Fedora).
    QSettings panel_settings("Forest", "Panel");
    panel_settings.beginGroup("plugins");
    foreach (QString child, panel_settings.childGroups()){
        QString path = panel_settings.value(child + "/path").toString();
        if (path.startsWith("/usr/lib/forest/"))
            panel_settings.setValue(child + "/path", QString(FOREST_LIB_DIR) + path.mid(15));
    }
    panel_settings.endGroup();
}

void SettingsUpgradeManager::upgrade_0_10_0(){
    // forest-settings loads every module in <libdir>/forest/settings now.
    settings->beginGroup("plugins");
    foreach (QString child, settings->childGroups())
        if (settings->value(child + "/settings-only", false).toBool())
            settings->remove(child);
    settings->endGroup();

    // Sensor Monitor keys sensors by chip + label now; the shown sensor and bar choices reset.
    QSettings sensors("Forest", "Temperature Monitor");
    foreach (QString key, sensors.childKeys())
        if (key.endsWith("Barenabled") || key == "ChipIndex" || key == "sensorlist" || key == "displayheight")
            sensors.remove(key);

    // Hold-to-repeat for the shipped volume/brightness hotkeys; customized actions are left alone.
    const QStringList repeating = {
        "DBUS:bus=Session,service=org.forest,path=/org/forest/panel/volume,method=volumeup",
        "DBUS:bus=Session,service=org.forest,path=/org/forest/panel/volume,method=volumedown",
        "bash -c \"brightnessctl set $((`brightnessctl get`-100))\"",
        "bash -c \"brightnessctl set $((`brightnessctl get`+100))\"",
    };
    settings->beginGroup("hotkeys");
    foreach (QString child, settings->childGroups())
        if (repeating.contains(settings->value(child + "/action").toString()) && !settings->contains(child + "/repeat"))
            settings->setValue(child + "/repeat", true);
    settings->endGroup();
}
