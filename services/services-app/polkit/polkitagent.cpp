// SPDX-License-Identifier: LGPL-3.0-or-later

#include "polkitagent.h"

using namespace PolkitQt1;
using namespace Agent;

polkitagent::polkitagent(QObject *parent) : PolkitQt1::Agent::Listener(parent), auth_in_progress(false){
    UnixSessionSubject session(getpid());
    registerListener(session, "/org/forest/PolicyKit1/AuthenticationAgent");
}

polkitagent::~polkitagent(){
    if (pkwidget){
        pkwidget->blockSignals(true);
        pkwidget->deleteLater();
    }
}

void polkitagent::initiateAuthentication(
        const QString &actionId, const QString &message, const QString &iconName, const Details &details, const QString &cookie,
        const Identity::List &identities, AsyncResult *result){

    if (auth_in_progress){
        QMessageBox::information(nullptr, "Authentication", "Authentication is already in progress. Please try again later.");
        return;
    }
    auth_in_progress = true;

    if (pkwidget){delete pkwidget;}
    pkwidget = new polkitdialog(actionId, message, iconName, details, identities);

    auth_cookie = cookie;
    auth_result = result;

    foreach (Identity i, identities) initiate_session(i);
}

bool polkitagent::initiateAuthenticationFinish(){
    auth_in_progress = false;
    return true;
}

void polkitagent::cancelAuthentication(){
    auth_in_progress = false;
}

void polkitagent::request(const QString &request, bool echo){
    Session *session = qobject_cast<Session *>(sender());
    Identity identity = session_identities[session];
    pkwidget->setPrompt(identity, request, echo);
    if (pkwidget->exec()){
        session->setResponse(pkwidget->response());
        return;
    }
    auth_canceled = true;
    session->cancel();
}

void polkitagent::completed(bool gainedAuthorization){
    Session *session = qobject_cast<Session *>(sender());
    if (!gainedAuthorization && !auth_canceled){
        pkwidget->setError("Authentication failed. Please try again.");
        initiate_session(session_identities[session]);
        return;
    }
    session->result()->setCompleted();
    session->disconnect();
    foreach (Session *s, session_identities.keys()){
        if (!s) continue;
        s->disconnect();
        s->deleteLater();
    }
    session_identities.clear();
    auth_in_progress = false;
    auth_canceled = false;
    auth_cookie = "";
    auth_result = nullptr;
}

void polkitagent::showError(const QString &text){
    QMessageBox::warning(nullptr, tr("PolicyKit Error"), text);
}

void polkitagent::showInfo(const QString &text){
    QMessageBox::information(nullptr, tr("PolicyKit Information"), text);
}

void polkitagent::initiate_session(Identity session_identity){
    Session *session = new Session(session_identity, auth_cookie, auth_result);
    session_identities[session] = session_identity;
    connect(session, SIGNAL(request(QString, bool)), this, SLOT(request(QString, bool)));
    connect(session, SIGNAL(completed(bool)), this, SLOT(completed(bool)));
    connect(session, SIGNAL(showError(QString)), this, SLOT(showError(QString)));
    connect(session, SIGNAL(showInfo(QString)), this, SLOT(showInfo(QString)));
    session->initiate();
}
