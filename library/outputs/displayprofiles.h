// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYPROFILES_H
#define DISPLAYPROFILES_H

#include <QDateTime>
#include <QSet>

#include "outputtypes.h"

struct DisplayProfile {
    QString id; // uuid without braces
    QString name;
    QDateTime lastUsed;
    QString primary; // connector
    OutputLayout outputs; // every output connected at save time, disabled ones too

    QSet<QString> outputSet() const;
};

// Displays.conf. Only the displays service writes it.
class DisplayProfiles {
public:
    void load();
    void save() const;

    QList<DisplayProfile> profiles() const{return m_profiles;}
    const DisplayProfile *find(const QString &id) const;
    DisplayProfile *find(const QString &id);
    void add(const DisplayProfile &profile);
    // Clears active if it was the active one.
    void remove(const QString &id);

    QString active() const{return m_active;}
    void setActive(const QString &id){m_active = id;}

    // Layout JSON to restore if the daemon stops mid-confirm; empty otherwise.
    QString pendingRevert() const{return m_pendingRevert;}
    void setPendingRevert(const QString &json){m_pendingRevert = json;}

    // Profiles whose output set equals the connected set, most recently used first.
    QList<DisplayProfile> matching(const OutputState &state) const;

    static bool matches(const DisplayProfile &profile, const OutputState &state);

    // Maps the profile onto the live heads by identity key and picks the closest
    // available mode. Its disconnected outputs are dropped; live heads it doesn't
    // mention are added disabled.
    static OutputLayout resolve(const DisplayProfile &profile, const OutputState &state);
    // Profile outputs with no live head.
    static OutputLayout disconnected(const DisplayProfile &profile, const OutputState &state);
    // The profile's primary as a connector of `layout` (matched by identity key), or empty.
    static QString resolvePrimary(const DisplayProfile &profile, const OutputLayout &layout);

    static QList<DisplayProfile> sortedByName(QList<DisplayProfile> profiles);

    // Default profile name from the enabled outputs, e.g. "DP-2 + HDMI-A-1".
    static QString defaultName(const OutputLayout &layout);

private:
    QList<DisplayProfile> m_profiles;
    QString m_active;
    QString m_pendingRevert;
};

#endif // DISPLAYPROFILES_H
