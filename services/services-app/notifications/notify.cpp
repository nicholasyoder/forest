// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notify.h"

namespace {
constexpr int max_visible = 5;
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
    const bool resident = hints.value("resident").toBool();
    if (replaces_id != 0 && popuphash.contains(replaces_id)) {
        notifypopup *popup = popuphash.value(replaces_id); // updated in place: no remap, keeps its stack slot
        popup->update_content(app_name, summary, body, app_icon, actions, expire_timeout);
        popup->set_resident(resident);
        return replaces_id;
    }

    const uint id = next_id++;
    if (next_id == 0)
        next_id = 1; // 0 is reserved by the spec

    notifypopup *npop = new notifypopup(app_name, summary, body, app_icon, actions, expire_timeout, id);
    npop->set_resident(resident);
    connect(npop, &notifypopup::readyToClose, this, &notify::close_popup);
    connect(npop, &notifypopup::actionInvoked, this, [this, npop](uint id, const QString &key) {
        emit ActionInvoked(id, key);
        if (!npop->resident())
            close_popup(id, Dismissed);
    });
    connect(npop, &notifypopup::heightChanged, this, &notify::restack);
    popuphash[id] = npop;
    stack.append(npop);
    restack();
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
    restack();
}

// Lay popups out from the corner inward, each past the ones before it. Only the
// oldest max_visible are shown; the rest wait (timers unstarted) for a free slot.
void notify::restack()
{
    if (restacking) { // show() resizes, which re-enters here
        restack_again = true;
        return;
    }
    restacking = true;
    do {
        restack_again = false;
        int offset = 0, shown = 0;
        for (notifypopup *popup : std::as_const(stack)) {
            if (shown++ >= max_visible)
                break;
            popup->set_stack_offset(offset);
            if (!held && !popup->isVisible())
                popup->show();
            offset += popup->height() - popup->stack_overlap(); // margins overlap so the gap equals the screen-edge gap
        }
    } while (restack_again);
    restacking = false;
}

bool notify::close_popup(uint id, uint reason){
    if (!popuphash.contains(id))
        return false;
    remove_popup(id);
    emit NotificationClosed(id, reason);
    return true;
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
