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

#include "polkitagent.h"

polkitagent::polkitagent(QObject *parent) : PolkitQt1::Agent::Listener(parent), m_inProgress(false){
    PolkitQt1::UnixSessionSubject session(getpid());
    registerListener(session, "/org/forest/PolicyKit1/AuthenticationAgent");
}

polkitagent::~polkitagent(){
    if (pkwidget){
        pkwidget->blockSignals(true);
        pkwidget->deleteLater();
    }
}


void polkitagent::initiateAuthentication(const QString &actionId, const QString &message, const QString &iconName,
        const PolkitQt1::Details &details, const QString &cookie, const PolkitQt1::Identity::List &identities, PolkitQt1::Agent::AsyncResult *result){

    if (m_inProgress){
        QMessageBox::information(nullptr, tr("PolicyKit Information"), "Another authentization in progress. Please try it again later");
        return;
    }

    m_inProgress = true;
    m_SessionIdentity.clear();

    if (pkwidget){
        delete pkwidget;
        pkwidget = nullptr;
    }
    pkwidget = new polkitdialog(actionId, message, iconName, details, identities);

    m_cookie = cookie;
    m_identities = identities;
    m_result = result;

    doAuth();
}

bool polkitagent::initiateAuthenticationFinish(){
    m_inProgress = false;
    return true;
}

void polkitagent::cancelAuthentication(){
    m_inProgress = false;
}

void polkitagent::request(const QString &request, bool echo){
    PolkitQt1::Agent::Session *session = qobject_cast<PolkitQt1::Agent::Session *>(sender());
    Q_ASSERT(session);
    Q_ASSERT(pkwidget);

    PolkitQt1::Identity identity = m_SessionIdentity[session];
    pkwidget->setPrompt(identity, request, echo);
    if (pkwidget->exec()){
        session->setResponse(pkwidget->response());
    }
    else{
        canceled = true;
        session->cancel();
    }
}

void polkitagent::completed(bool gainedAuthorization){
    PolkitQt1::Agent::Session * session = qobject_cast<PolkitQt1::Agent::Session *>(sender());
    Q_ASSERT(session);

    if (!gainedAuthorization && !canceled){
        QMessageBox::information(nullptr, tr("Authorization Failed"), tr("Authorization failed for some reason"));
    }

    session->result()->setCompleted();

    delete session;

    m_inProgress = false;
    canceled = false;
}

void polkitagent::showError(const QString &text){
    QMessageBox::warning(nullptr, tr("PolicyKit Error"), text);
}

void polkitagent::showInfo(const QString &text){
    QMessageBox::information(nullptr, tr("PolicyKit Information"), text);
}

void polkitagent::doAuth(){
    foreach (PolkitQt1::Identity i, m_identities){
        PolkitQt1::Agent::Session *session;
        session = new PolkitQt1::Agent::Session(i, m_cookie, m_result);
        Q_ASSERT(session);
        m_SessionIdentity[session] = i;
        connect(session, SIGNAL(request(QString, bool)), this, SLOT(request(QString, bool)));
        connect(session, SIGNAL(completed(bool)), this, SLOT(completed(bool)));
        connect(session, SIGNAL(showError(QString)), this, SLOT(showError(QString)));
        connect(session, SIGNAL(showInfo(QString)), this, SLOT(showInfo(QString)));
        session->initiate();
    }
}
