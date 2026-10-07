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
class IdentifyCards;

// Display profile daemon: auto-picks a profile at startup and on hotplug,
// applies layouts and profiles, and saves profiles for D-Bus callers (hotkeys,
// the Displays settings page). The active profile is whichever equals the live
// layout, if any.
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
    // Applies a layout (layoutToJson), after confirmation if it can leave a
    // screen dark. Saves nothing. False if refused.
    bool applyLayout(const QString &json);
    void keepLayout();
    void revertLayout();
    // Neither applies anything. False / empty if refused.
    bool saveProfile(const QString &id, const QString &json);
    QString saveProfileAs(const QString &name, const QString &json);
    bool renameProfile(const QString &id, const QString &name);
    void deleteProfile(const QString &id);
    // Shows each enabled output's name on its screen for a few seconds.
    void identify();

signals:
    void profilesChanged();
    void activeProfileChanged(const QString &id);
    void primaryChanged(const QString &name);
    void confirmPending();
    void layoutKept();
    void layoutReverted();
    void applyFailed();
    // Forest.conf [hotkeys] changed (profile renamed or deleted).
    void hotkeysChanged();

private:
    void onStateChanged();
    void autoPick();
    void apply(const DisplayProfile &profile);
    void markActive(const QString &id);
    // Re-derives the active profile from the live layout.
    void syncActive();
    bool isLive(const DisplayProfile &profile) const;
    // Completes a D-Bus layout against the live heads. False if unusable.
    bool parseLayout(const QString &json, const char *caller, OutputLayout *layout, QString *primary) const;
    void setPrimary(const QString &name);
    // Removes the [hotkeys] entries that apply profile `id`, or renames them
    // when `newName` is set (only descriptions still at the default).
    void updateProfileHotkeys(const QString &id, const QString &oldName, const QString &newName = QString());
    // Applies `target` (connectors may have changed since) and `primary`; emits layoutReverted.
    void restore(const OutputLayout &target, const QString &primary);
    void startConfirm(const OutputLayout &revertTo, const QString &revertPrimary, const OutputLayout &applied);
    // Clears a pending confirmation without reverting.
    void endConfirm();
    static QString currentPrimary();

    static constexpr int kConfirmSeconds = 15;

    OutputManager *manager = nullptr;
    DisplayProfiles profiles;
    QSet<QString> connectedKeys;
    bool started = false;
    int applying = 0; // applies in flight; the live layout is in flux
    RunOnce hotplugDebounce{500};

    bool pending = false;
    OutputLayout revertTarget; // live layout before the first unconfirmed apply
    QString revertPrimary;
    QTimer revertTimer;
    QDeadlineTimer revertDeadline;
    ConfirmCards *cards = nullptr;
    IdentifyCards *identifyCards = nullptr;
};

#endif // DISPLAYS_H
