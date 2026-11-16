// SPDX-License-Identifier: LGPL-3.0-or-later

#include "hotkey.h"

globalhotkey::globalhotkey(const QString &id, const QString &description, const QString &trigger, HK_Type type)
    : hotkey_id(id), hotkey_description(description), hotkey_trigger(trigger), hotkey_type(type) {
    qDebug() << "Add Global Hotkey:" << id << trigger;
    // Repeats run without a token: tokens are single-use.
    connect(&repeatTimer, &QTimer::timeout, this, [this]() { exec(); });
    // A lost Deactivated must not leave the action repeating forever.
    repeatCap.setSingleShot(true);
    repeatCap.setInterval(kMaxRepeatMs);
    connect(&repeatCap, &QTimer::timeout, this, &globalhotkey::release);
}

void globalhotkey::setRepeatInterval(int intervalMs){
    repeatTimer.stop();
    repeatTimer.setInterval(qMax(intervalMs, 0));
}

void globalhotkey::press(const QString &activationToken){
    exec(activationToken);
    if (repeatTimer.interval() > 0) {
        repeatTimer.start();
        repeatCap.start();
    }
}

void globalhotkey::release(){
    repeatTimer.stop();
    repeatCap.stop();
}

void globalhotkey::exec(const QString &activationToken){
    if(hotkey_type == Type_Exec){
        QStringList args = QProcess::splitCommand(shcommand);
        if (!args.isEmpty()) {
            QProcess process;
            process.setProgram(args.takeFirst());
            process.setArguments(args);
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.remove("XDG_ACTIVATION_TOKEN");
            if (!activationToken.isEmpty())
                env.insert("XDG_ACTIVATION_TOKEN", activationToken);
            process.setProcessEnvironment(env);
            process.startDetached();
        }
    }
    else{
        QDBusConnection bus = dbusaction.systemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();
        QDBusMessage call = QDBusMessage::createMethodCall(dbusaction.service, dbusaction.path,
                                                           dbusaction.interface, dbusaction.method);
        if (!dbusaction.arg.isEmpty()) call << dbusaction.arg;
        if (!activationToken.isEmpty() && hotkeyconfig::takesActivationToken(dbusaction)) call << activationToken;
        auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(call), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *w) {
            if (w->isError()) qWarning() << "globalhotkey:" << hotkey_id << "D-Bus call failed:" << w->error().message();
            w->deleteLater();
        });
    }
}
