// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkey.h"
#include "keysym_table.h"

#include <xkbcommon/xkbcommon.h>

globalhotkey::globalhotkey(const QString &id, const QString &description, const QKeySequence &sequence, HK_Type type)
    : hotkey_id(id), hotkey_description(description), keyseq(sequence), hotkey_type(type) {
    qDebug() << "Add Global Hotkey:" << id << sequence.toString();
}

void globalhotkey::setDbusInfo(QString service, QString path,QString interface, QString method, QString bus){
    dbusservice = service;
    dbuspath = path;
    dbusinterface = interface;
    dbusmethod = method;
    dbusbus = bus;
}

void globalhotkey::exec(){
    if(hotkey_type == Type_Exec){
        QStringList args = QProcess::splitCommand(shcommand);
        if (!args.isEmpty()) {
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        }
    }
    else{
        if (dbusbus == "Session" || dbusbus == ""){
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::sessionBus());
            if (!dbus.isValid()){
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }
        else{
            QDBusInterface dbus(dbusservice, dbuspath, dbusinterface, QDBusConnection::systemBus());
            if (!dbus.isValid()){
                qDebug() << "QDBusInterface is not valid!";
                return ;
            }
            dbus.call(dbusmethod);
        }
    }
}

QString globalhotkey::triggerString() const {
    if (keyseq.isEmpty()) {
        return QString();
    }

    const QKeyCombination combo = keyseq[0];
    const Qt::KeyboardModifiers mods = combo.keyboardModifiers();
    const Qt::Key key = combo.key();

    // Bare-Meta-tap - foresthotkeys::loadhotkeys() builds this exact
    // QKeySequence(Qt::Key_Meta) (key with no modifiers) for the "Meta"
    // sentinel in Forest.conf. The shortcuts-spec grammar has no syntax for
    // a modifier used as a "key" like Qt does here, so this maps onto
    // biome/core/keybindings.cpp's parse_trigger() modifier-only extension
    // instead: the trigger string is just the modifier name on its own.
    if (key == Qt::Key_Meta && mods == Qt::NoModifier) {
        return QStringLiteral("LOGO");
    }

    QStringList parts;
    if (mods & Qt::ControlModifier) parts << QStringLiteral("CTRL");
    if (mods & Qt::AltModifier) parts << QStringLiteral("ALT");
    if (mods & Qt::ShiftModifier) parts << QStringLiteral("SHIFT");
    if (mods & Qt::MetaModifier) parts << QStringLiteral("LOGO");

    // Media/brightness/function keys first: the table maps Qt::Key to xkb
    // keysyms (same values as libxkbcommon's), so a cast is enough.
    xkb_keysym_t keysym = XKB_KEY_NoSymbol;
    int i = 0;
    while (KeyTbl[i]) {
        if (KeyTbl[i + 1] == static_cast<uint>(key)) {
            keysym = static_cast<xkb_keysym_t>(KeyTbl[i]);
            break;
        }
        i += 2;
    }

    // Not in the table (ordinary letter/digit/punctuation): resolve by name.
    if (keysym == XKB_KEY_NoSymbol) {
        const QByteArray name = QKeySequence(key).toString().toUtf8();
        keysym = xkb_keysym_from_name(name.constData(), XKB_KEYSYM_CASE_INSENSITIVE);
    }

    if (keysym == XKB_KEY_NoSymbol) {
        qWarning() << "globalhotkey: could not resolve a keysym for" << keyseq.toString();
        return QString();
    }

    char name[64];
    if (xkb_keysym_get_name(keysym, name, sizeof(name)) < 0) {
        qWarning() << "globalhotkey: xkb_keysym_get_name failed for keysym" << keysym;
        return QString();
    }

    parts << QString::fromUtf8(name);
    return parts.join('+');
}
