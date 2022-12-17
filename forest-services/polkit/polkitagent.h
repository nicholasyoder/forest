/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 * based on razorqt-policykit
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#ifndef POLKITAGENT_H
#define POLKITAGENT_H

#include <QWidget>
#include <QMessageBox>
#include <QDebug>

#include <polkit-qt5-1/polkitqt1-agent-session.h>
#include <polkit-qt5-1/polkitqt1-identity.h>
#include <polkit-qt5-1/polkitqt1-details.h>
#include <polkit-qt5-1/polkitqt1-agent-listener.h>
#include <polkit-qt5-1/polkitqt1-subject.h>

#include "polkitdialog.h"

class polkitagent : public PolkitQt1::Agent::Listener
{
    Q_OBJECT

public:
    polkitagent(QObject *parent = nullptr);
    ~polkitagent();

public slots:
    void initiateAuthentication(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details,
                                const QString &cookie, const PolkitQt1::Identity::List &identities, PolkitQt1::Agent::AsyncResult *result);

    bool initiateAuthenticationFinish();
    void cancelAuthentication();

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);
    void showError(const QString &text);
    void showInfo(const QString &text);

private:
    bool m_inProgress;
    bool canceled = false;
    polkitdialog * pkwidget = nullptr;
    QHash<PolkitQt1::Agent::Session*,PolkitQt1::Identity> m_SessionIdentity;
};

#endif // POLKITAGENT_H
