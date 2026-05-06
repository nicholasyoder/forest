// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSUPGRADEMANAGER_H
#define SETTINGSUPGRADEMANAGER_H

#include <QObject>
#include <QSettings>

class SettingsUpgradeManager : public QObject {
    Q_OBJECT
public:
    explicit SettingsUpgradeManager(QObject *parent = nullptr);
    void perform_upgrades();
private:
    QSettings *settings = nullptr;
    void load_defaults();

    void upgrade_0_7_9();
};

#endif // SETTINGSUPGRADEMANAGER_H
