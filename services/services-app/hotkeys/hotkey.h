// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEY_H
#define HOTKEY_H

#include <QtDBus>
#include <QKeySequence>
#include <QTimer>

#include "hotkeyconfig.h"

enum HK_Type
{
    Type_Exec,
    Type_Dbus
};

// One configured hotkey: its action, plus its portal id/description/trigger.
class globalhotkey : public QObject
{
    Q_OBJECT

public:
    globalhotkey(const QString &id, const QString &description, const QKeySequence &sequence, HK_Type type);

    // The Forest.conf group name, e.g. "item-0001".
    QString id() const { return hotkey_id; }

    QString description() const { return hotkey_description; }

    // Shortcuts-spec trigger, e.g. "CTRL+ALT+Return"; "LOGO" for bare Meta.
    QString triggerString() const;

public slots:
    void setDbusAction(const DBusHotkeyAction &action){dbusaction = action;}
    void setExecCommand(const QString &command){shcommand=command;}
    // intervalMs <= 0 disables repeat.
    void setRepeatInterval(int intervalMs);

    void exec(const QString &activationToken = QString());
    // Key down/up: exec() once, then again every repeat interval until release().
    void press(const QString &activationToken);
    void release();

private:
    QString hotkey_id;
    QString hotkey_description;
    QKeySequence keyseq;
    HK_Type hotkey_type;
    QString shcommand;
    DBusHotkeyAction dbusaction;
    QTimer repeatTimer;
};

#endif // HOTKEY_H
