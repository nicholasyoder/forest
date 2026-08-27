// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEY_H
#define HOTKEY_H

#include <QtDBus>
#include <QKeySequence>

enum HK_Type
{
    Type_Exec,
    Type_Dbus
};

// A single configured hotkey entry - just the DBus/exec action payload plus
// enough to build a shortcuts-spec trigger string for the portal. No longer
// grabs anything itself (see docs/phase4-plan.md's Workstream C): binding
// and dispatch both live one level up, in foresthotkeys/GlobalShortcutsPortal.
class globalhotkey : public QObject
{
    Q_OBJECT

public:
    globalhotkey(const QString &id, const QString &description, const QKeySequence &sequence, HK_Type type);

    // The portal shortcut id - reuses the QSettings group name
    // (foresthotkeys::loadhotkeys()'s "item-0001" etc.), already unique.
    QString id() const { return hotkey_id; }

    // Forest.conf's "description" value for this entry, passed through as
    // the portal shortcut's description vardict key.
    QString description() const { return hotkey_description; }

    // Builds the shortcuts-spec grammar trigger string
    // (biome/core/keybindings.cpp's parse_trigger()) from the stored
    // QKeySequence - e.g. "CTRL+ALT+Return", or "LOGO" alone for the
    // bare-Meta-tap binding.
    QString triggerString() const;

public slots:
    void setDbusInfo(QString service, QString path, QString interface, QString method, QString bus);
    void setExecCommand(const QString &command){shcommand=command;}

    void exec();

private:
    QString hotkey_id;
    QString hotkey_description;
    QKeySequence keyseq;
    HK_Type hotkey_type;
    QString shcommand;
    QString dbusservice;
    QString dbuspath;
    QString dbusinterface;
    QString dbusmethod;
    QString dbusbus;
};

#endif // HOTKEY_H
