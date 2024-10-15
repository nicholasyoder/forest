/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2.1+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2015 David Edmundson <davidedmundson@kde.org>
 *            2022 LXQt team
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#pragma once

#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QObject>

#include <xcb/xcb.h>
#include <memory>

class KSelectionOwner;
class SNIProxy;

namespace Xcb {
    class Atoms;
}

class FdoSelectionManager : public QObject //, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    FdoSelectionManager();
    ~FdoSelectionManager() override;

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);

private Q_SLOTS:
    void onClaimedOwnership();
    void onFailedToClaimOwnership();
    void onLostOwnership();

private:
    void init();
    bool addDamageWatch(xcb_window_t client);
    void dock(xcb_window_t embed_win);
    void undock(xcb_window_t client, bool vanished);
    void setSystemTrayVisual();

    uint8_t m_damageEventBase;

    xcb_connection_t *m_connection;
    QHash<xcb_window_t, u_int32_t> m_damageWatches;
    QHash<xcb_window_t, SNIProxy *> m_proxies;
    std::unique_ptr<Xcb::Atoms> m_atoms;
    KSelectionOwner *m_selectionOwner;
};
