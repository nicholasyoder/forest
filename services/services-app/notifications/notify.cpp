// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notify.h"

namespace {
constexpr char startup_property[] = "forestStartupOverlay"; // set by forest while its startup overlay is up
}

notify::notify()
{
}

notify::~notify()
{
}

void notify::setup()
{
    held = qApp->property(startup_property).toBool();
    if (held)
        qApp->installEventFilter(this);
    new notifyadapter(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerObject("/org/freedesktop/Notifications", this))
        qCritical() << "Failed to register /org/freedesktop/Notifications on DBus:" << connection.lastError().message();
    if (!connection.registerService("org.freedesktop.Notifications"))
        qCritical() << "Failed to register org.freedesktop.Notifications on DBus:" << connection.lastError().message();
}

uint notify::show_notification(
        const QString &app_name,
        uint replaces_id,
        const QString &app_icon,
        const QString &summary,
        const QString &body,
        const QStringList &actions,
        const QVariantMap &hints,
        int expire_timeout)
{
    uint id;
    int slot = -1;
    if (replaces_id != 0 && popuphash.contains(replaces_id)) {
        id = replaces_id;
        slot = stack.indexOf(popuphash.value(id));
        remove_popup(id); // a replaced notification isn't "closed", so no signal
    } else {
        id = next_id++;
        if (next_id == 0)
            next_id = 1; // 0 is reserved by the spec
    }

    bool resident = hints.value("resident").toBool();
    notifypopup *npop = new notifypopup(app_name, summary, body, app_icon, actions, expire_timeout, id);
    connect(npop, &notifypopup::readyToClose, this, &notify::close_popup);
    connect(npop, &notifypopup::actionInvoked, this, [this, resident](uint id, const QString &key) {
        emit ActionInvoked(id, key);
        if (!resident)
            close_popup(id, Dismissed);
    });
    connect(npop, &notifypopup::heightChanged, this, &notify::restack);
    popuphash[id] = npop;
    stack.insert(slot < 0 ? stack.size() : qMin(slot, int(stack.size())), npop);
    restack();
    if (!held)
        npop->show();
    return id;
}

bool notify::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp && event->type() == QEvent::DynamicPropertyChange
        && static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName() == startup_property
        && !qApp->property(startup_property).toBool())
        show_held();
    return QObject::eventFilter(watched, event);
}

void notify::show_held()
{
    held = false;
    qApp->removeEventFilter(this);
    for (notifypopup *popup : std::as_const(stack))
        popup->show();
}

// Lay popups out from the corner inward, each past the ones before it.
void notify::restack()
{
    int offset = 0;
    for (notifypopup *popup : std::as_const(stack)) {
        popup->set_stack_offset(offset);
        offset += popup->height() - popup->stack_overlap(); // margins overlap so the gap equals the screen-edge gap
    }
}

void notify::close_popup(uint id, uint reason){
    if (!popuphash.contains(id))
        return;
    remove_popup(id);
    emit NotificationClosed(id, reason);
}

void notify::remove_popup(uint id){
    notifypopup *npop = popuphash.take(id);
    if (npop) {
        stack.removeOne(npop);
        npop->close();
        npop->deleteLater();
        restack();
    }
}
