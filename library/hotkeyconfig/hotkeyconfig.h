// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTKEYCONFIG_H
#define HOTKEYCONFIG_H

#include <QKeySequence>
#include <QList>
#include <QSettings>
#include <QString>

#include <optional>

// A [hotkeys] action in Forest.conf of the form
// "DBUS:bus=Session,service=…,path=…,interface=…,method=…[,arg=…]".
struct DBusHotkeyAction {
    bool systemBus = false;
    QString service;
    QString path;
    QString interface;
    QString method;
    QString arg; // optional single string argument
    // Built-ins only, not stored: the call takes an xdg-activation token as its last argument.
    bool activationToken = false;

    // Same call; an empty interface on either side matches any.
    bool sameTarget(const DBusHotkeyAction &other) const;
};

struct BuiltinHotkeyAction {
    QString description;
    DBusHotkeyAction action;
    QString note; // e.g. what must be running for the call to work
};

namespace hotkeyconfig {

// Per-hotkey keys under [hotkeys]/<item>: re-run the action every interval while held.
inline constexpr char kRepeatKey[] = "repeat";
inline constexpr char kRepeatIntervalKey[] = "repeat_interval";
inline constexpr int kDefaultRepeatIntervalMs = 100;

// Per-hotkey key under [hotkeys]/<item>: shortcuts-spec trigger, modifiers in
// caps and the keysym name lowercased ("CTRL+escape", "xf86audiomute"); "LOGO"
// alone is a bare Meta tap. Replaces the old Qt-style "keysequence".
inline constexpr char kTriggerKey[] = "trigger";
inline constexpr char kLegacyKeysequenceKey[] = "keysequence";

// Trigger for a captured key (xkb keysym, e.g. QKeyEvent::nativeVirtualKey());
// empty if the keysym has no name.
QString triggerFromKeysym(Qt::KeyboardModifiers mods, quint32 keysym);
// Converts an old "keysequence" value; empty if it can't be resolved.
QString triggerFromLegacy(const QString &keysequence);
// "CTRL+xf86audiomute" -> "Ctrl+XF86AudioMute".
QString triggerDisplay(const QString &trigger);
// Why Biome would refuse `trigger` (empty parts, bare key); empty if it's fine.
QString triggerRefusal(const QString &trigger);
// The current group's trigger, falling back to a converted "keysequence".
QString readTrigger(const QSettings &settings);

// nullopt if `action` isn't a DBUS: action.
std::optional<DBusHotkeyAction> parseDBusAction(const QString &action);
QString formatDBusAction(const DBusHotkeyAction &action);

// Fixed user-facing org.forest calls; display profiles are listed separately.
QList<BuiltinHotkeyAction> builtinActions();

// Whether `action` is a built-in that takes an xdg-activation token.
bool takesActivationToken(const DBusHotkeyAction &action);

// Built-in actions on org.forest /org/forest/displays.
DBusHotkeyAction displayProfileAction(const QString &profileId);
DBusHotkeyAction nextDisplayProfileAction();
QString displayProfileDescription(const QString &profileName);

}

#endif // HOTKEYCONFIG_H
