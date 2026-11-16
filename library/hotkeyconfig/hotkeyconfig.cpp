// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkeyconfig.h"

#include <QHash>
#include <QRegularExpression>
#include <QStringList>

#include <xkbcommon/xkbcommon.h>

static const QString kPrefix = QStringLiteral("DBUS:");

bool DBusHotkeyAction::sameTarget(const DBusHotkeyAction &other) const{
    return systemBus == other.systemBus && service == other.service && path == other.path
        && method == other.method && arg == other.arg
        && (interface.isEmpty() || other.interface.isEmpty() || interface == other.interface);
}

namespace hotkeyconfig {

std::optional<DBusHotkeyAction> parseDBusAction(const QString &action){
    if (!action.startsWith(kPrefix)) return std::nullopt;

    QHash<QString, QString> options;
    for (const QString &s : action.mid(kPrefix.size()).split(',')){
        const int eq = s.indexOf('='); // the first: an arg may contain '='
        if (eq > 0) options[s.left(eq)] = s.mid(eq + 1);
    }

    DBusHotkeyAction a;
    a.systemBus = options.value("bus") == "System";
    a.service = options.value("service");
    a.path = options.value("path");
    a.interface = options.value("interface");
    a.method = options.value("method");
    a.arg = options.value("arg");
    return a;
}

QString formatDBusAction(const DBusHotkeyAction &a){
    QStringList parts = {
        "bus=" + QString(a.systemBus ? "System" : "Session"),
        "service=" + a.service,
        "path=" + a.path,
        "interface=" + a.interface,
        "method=" + a.method,
    };
    if (!a.arg.isEmpty()) parts << "arg=" + a.arg;
    return kPrefix + parts.join(',');
}

// No interface: the panel plugins export Qt's generated local.* names.
static DBusHotkeyAction forestAction(const QString &path, const QString &method, bool activationToken = false){
    DBusHotkeyAction a;
    a.service = "org.forest";
    a.path = path;
    a.method = method;
    a.activationToken = activationToken;
    return a;
}

QList<BuiltinHotkeyAction> builtinActions(){
    const QString menu = "/org/forest/panel/mainmenu";
    const QString volume = "/org/forest/panel/volume";
    const QString volumeNote = "Needs the Volume panel plugin";
    return {
        {"Toggle main menu", forestAction(menu, "togglemenu", true), {}},
        {"Show main menu", forestAction(menu, "showmenu", true), {}},
        {"Show desktop", forestAction("/org/forest/showdesktop", "toggle"), {}},
        {"Volume up", forestAction(volume, "volumeup"), volumeNote},
        {"Volume down", forestAction(volume, "volumedown"), volumeNote},
        {"Toggle mute", forestAction(volume, "togglemuted"), volumeNote},
    };
}

bool takesActivationToken(const DBusHotkeyAction &action){
    for (const BuiltinHotkeyAction &builtin : builtinActions())
        if (builtin.action.activationToken && builtin.action.sameTarget(action))
            return true;
    return false;
}

static DBusHotkeyAction displaysAction(const QString &method, const QString &arg = QString()){
    DBusHotkeyAction a;
    a.service = "org.forest";
    a.path = "/org/forest/displays";
    a.interface = "org.forest.displays";
    a.method = method;
    a.arg = arg;
    return a;
}

DBusHotkeyAction displayProfileAction(const QString &profileId){
    return displaysAction("applyProfile", profileId);
}

DBusHotkeyAction nextDisplayProfileAction(){
    return displaysAction("nextProfile");
}

QString displayProfileDescription(const QString &profileName){
    return "Display profile: " + profileName;
}


namespace {

QString keysymName(xkb_keysym_t keysym){
    char name[64];
    if (xkb_keysym_get_name(keysym, name, sizeof(name)) < 0) return QString();
    return QString::fromLatin1(name);
}

QString modifierParts(Qt::KeyboardModifiers mods){
    QStringList parts;
    if (mods & Qt::ControlModifier) parts << QStringLiteral("CTRL");
    if (mods & Qt::AltModifier) parts << QStringLiteral("ALT");
    if (mods & Qt::ShiftModifier) parts << QStringLiteral("SHIFT");
    if (mods & Qt::MetaModifier) parts << QStringLiteral("LOGO");
    return parts.join('+');
}

// Non-Latin-1 Qt keys an old "keysequence" can hold, as xkb keysym names.
const QHash<int, const char *> &legacyKeyNames(){
    static const QHash<int, const char *> names = {
        {Qt::Key_Escape, "Escape"}, {Qt::Key_Tab, "Tab"}, {Qt::Key_Backtab, "Tab"},
        {Qt::Key_Backspace, "BackSpace"}, {Qt::Key_Return, "Return"}, {Qt::Key_Enter, "KP_Enter"},
        {Qt::Key_Insert, "Insert"}, {Qt::Key_Delete, "Delete"}, {Qt::Key_Pause, "Pause"},
        {Qt::Key_Print, "Print"}, {Qt::Key_SysReq, "Sys_Req"}, {Qt::Key_Home, "Home"},
        {Qt::Key_End, "End"}, {Qt::Key_Left, "Left"}, {Qt::Key_Up, "Up"}, {Qt::Key_Right, "Right"},
        {Qt::Key_Down, "Down"}, {Qt::Key_PageUp, "Prior"}, {Qt::Key_PageDown, "Next"},
        {Qt::Key_Menu, "Menu"}, {Qt::Key_ScrollLock, "Scroll_Lock"},
        {Qt::Key_VolumeUp, "XF86AudioRaiseVolume"}, {Qt::Key_VolumeDown, "XF86AudioLowerVolume"},
        {Qt::Key_VolumeMute, "XF86AudioMute"}, {Qt::Key_MediaPlay, "XF86AudioPlay"},
        {Qt::Key_MediaStop, "XF86AudioStop"}, {Qt::Key_MediaPrevious, "XF86AudioPrev"},
        {Qt::Key_MediaNext, "XF86AudioNext"}, {Qt::Key_MediaTogglePlayPause, "XF86AudioPlay"},
        {Qt::Key_MicMute, "XF86AudioMicMute"}, {Qt::Key_MonBrightnessUp, "XF86MonBrightnessUp"},
        {Qt::Key_MonBrightnessDown, "XF86MonBrightnessDown"},
        {Qt::Key_KeyboardBrightnessUp, "XF86KbdBrightnessUp"},
        {Qt::Key_KeyboardBrightnessDown, "XF86KbdBrightnessDown"},
        {Qt::Key_HomePage, "XF86HomePage"}, {Qt::Key_Search, "XF86Search"},
        {Qt::Key_LaunchMail, "XF86Mail"}, {Qt::Key_Calculator, "XF86Calculator"},
        {Qt::Key_Sleep, "XF86Sleep"}, {Qt::Key_PowerOff, "XF86PowerOff"},
        {Qt::Key_WLAN, "XF86WLAN"}, {Qt::Key_Bluetooth, "XF86Bluetooth"},
        {Qt::Key_TouchpadToggle, "XF86TouchpadToggle"},
    };
    return names;
}

// Qt's readable name ("Volume Mute") for an XF86 keysym we know; empty otherwise.
QString xf86DisplayName(xkb_keysym_t keysym){
    int best = 0; // lowest Qt key wins where two map to one keysym
    for (auto it = legacyKeyNames().cbegin(); it != legacyKeyNames().cend(); ++it)
        if (xkb_keysym_from_name(it.value(), XKB_KEYSYM_NO_FLAGS) == keysym && (!best || it.key() < best))
            best = it.key();
    return best ? QKeySequence(best).toString(QKeySequence::NativeText) : QString();
}

} // namespace

QString triggerFromKeysym(Qt::KeyboardModifiers mods, quint32 keysym){
    if (keysym == XKB_KEY_ISO_Left_Tab) keysym = XKB_KEY_Tab; // Shift+Tab
    const QString name = keysymName(xkb_keysym_to_lower(keysym)).toLower();
    if (name.isEmpty()) return QString();
    const QString modifiers = modifierParts(mods & (Qt::ControlModifier | Qt::AltModifier
                                                    | Qt::ShiftModifier | Qt::MetaModifier));
    return modifiers.isEmpty() ? name : modifiers + '+' + name;
}

QString triggerFromLegacy(const QString &keysequence){
    if (keysequence.isEmpty()) return QString();
    const QKeySequence seq(keysequence);
    if (seq.isEmpty()) return QString();

    const QKeyCombination combo = seq[0];
    const Qt::Key key = combo.key();
    if (key == Qt::Key_Meta && combo.keyboardModifiers() == Qt::NoModifier) return QStringLiteral("LOGO");

    quint32 keysym = XKB_KEY_NoSymbol;
    if (key >= Qt::Key_F1 && key <= Qt::Key_F35)
        keysym = XKB_KEY_F1 + (key - Qt::Key_F1);
    else if (key >= 0x20 && key <= 0xff)
        keysym = key; // Latin-1 keysyms equal their code points
    else if (const char *name = legacyKeyNames().value(key, nullptr))
        keysym = xkb_keysym_from_name(name, XKB_KEYSYM_NO_FLAGS);
    if (keysym == XKB_KEY_NoSymbol) return QString();
    return triggerFromKeysym(combo.keyboardModifiers(), keysym);
}

QString triggerDisplay(const QString &trigger){
    const QStringList parts = trigger.split('+');
    if (parts.size() == 1 && trigger.compare("LOGO", Qt::CaseInsensitive) == 0) return QStringLiteral("Meta");
    QStringList out;
    for (int i = 0; i < parts.size() - 1; i++){
        const QString upper = parts.at(i).toUpper();
        out << (upper == "LOGO" ? QStringLiteral("Meta") : upper.at(0) + upper.mid(1).toLower());
    }
    const QString &last = parts.last();
    const xkb_keysym_t keysym = xkb_keysym_from_name(last.toUtf8().constData(), XKB_KEYSYM_CASE_INSENSITIVE);
    QString canonical = keysym == XKB_KEY_NoSymbol ? last : keysymName(keysym);
    if (canonical.startsWith("XF86")){
        const QString readable = xf86DisplayName(keysym);
        if (!readable.isEmpty()) canonical = readable;
    }
    out << (canonical.size() == 1 ? canonical.toUpper() : canonical);
    return out.join('+');
}

QString triggerRefusal(const QString &trigger){
    const QStringList parts = trigger.split('+');
    for (const QString &part : parts)
        if (part.isEmpty()) return QStringLiteral("The shortcut has an empty part.");
    if (parts.size() == 1 && trigger.compare("LOGO", Qt::CaseInsensitive) == 0) return QString();

    for (int i = 0; i < parts.size() - 1; i++){
        const QString mod = parts.at(i).toUpper();
        if (mod == "CTRL" || mod == "ALT" || mod == "LOGO") return QString();
    }
    static const QRegularExpression exempt(QStringLiteral("^(f\\d+|xf86.*|print|pause|break|sys_req)$"),
                                           QRegularExpression::CaseInsensitiveOption);
    if (exempt.match(parts.last()).hasMatch()) return QString();
    return QStringLiteral("Add Ctrl, Alt or Meta: a plain key (or Shift + key) can't be a global shortcut.");
}

QString readTrigger(const QSettings &settings){
    const QString trigger = settings.value(kTriggerKey).toString();
    if (!trigger.isEmpty()) return trigger;
    return triggerFromLegacy(settings.value(kLegacyKeysequenceKey).toString());
}

}
