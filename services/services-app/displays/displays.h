// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYS_H
#define DISPLAYS_H

#include <QObject>
#include <QSet>

#include "outputmanager.h"
#include "displayprofiles.h"
#include "miscutills/miscutills.h"

// Display profile daemon: auto-picks a profile at startup and on hotplug, and
// applies/saves them for D-Bus callers (hotkeys, the Displays settings page).
class Displays : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.forest.displays")

public:
    void setup();

public slots:
    // Ignored (logged) if the profile doesn't match the connected outputs.
    void applyProfile(const QString &id);
    // Cycles the matching profiles, by name.
    void nextProfile();
    // Snapshots the live layout as a new, active profile. Returns its id.
    QString saveCurrentAsProfile(const QString &name);

signals:
    void profilesChanged();
    void activeProfileChanged(const QString &id);

private:
    void onStateChanged();
    void autoPick();
    void apply(const DisplayProfile &profile);
    void markActive(const QString &id);

    OutputManager *manager = nullptr;
    DisplayProfiles profiles;
    QSet<QString> connectedKeys;
    bool started = false;
    RunOnce hotplugDebounce{500};
};

#endif // DISPLAYS_H
