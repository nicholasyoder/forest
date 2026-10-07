// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYS_H
#define DISPLAYS_H

#include <QDeadlineTimer>
#include <QObject>
#include <QSet>
#include <QTimer>

#include "outputmanager.h"
#include "displayprofiles.h"
#include "miscutills/miscutills.h"

class ConfirmCards;

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
    // Applies an edited layout (layoutToJson) and saves it to the active profile,
    // after confirmation if it can leave a screen dark. False if refused.
    bool applyLayout(const QString &json);
    void keepLayout();
    void revertLayout();

signals:
    void profilesChanged();
    void activeProfileChanged(const QString &id);
    void confirmPending();
    void layoutKept();
    void layoutReverted();
    void applyFailed();

private:
    void onStateChanged();
    void autoPick();
    void apply(const DisplayProfile &profile);
    void markActive(const QString &id);
    void saveActiveLayout(const OutputLayout &layout);
    void startConfirm(const OutputLayout &revertTo, const OutputLayout &applied);
    // Clears a pending confirmation without reverting or saving.
    void endConfirm();
    static QString defaultPrimary(const OutputLayout &layout);

    static constexpr int kConfirmSeconds = 15;

    OutputManager *manager = nullptr;
    DisplayProfiles profiles;
    QSet<QString> connectedKeys;
    bool started = false;
    RunOnce hotplugDebounce{500};

    bool pending = false;
    OutputLayout revertTarget; // live layout before the first unconfirmed apply
    OutputLayout pendingLayout;
    QTimer revertTimer;
    QDeadlineTimer revertDeadline;
    ConfirmCards *cards = nullptr;
};

#endif // DISPLAYS_H
