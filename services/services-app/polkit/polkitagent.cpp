// SPDX-License-Identifier: LGPL-3.0-or-later

#include "polkitagent.h"

#include <unistd.h>

using namespace PolkitQt1;
using namespace Agent;

polkitagent::polkitagent(QObject *parent) : PolkitQt1::Agent::Listener(parent){
    UnixSessionSubject session(getpid());
    if (!registerListener(session, "/org/forest/PolicyKit1/AuthenticationAgent"))
        qWarning() << "polkit: failed to register authentication agent";
}

polkitagent::~polkitagent(){
    pending_.clear();
    if (result_) finish();
}

void polkitagent::initiateAuthentication(
        const QString &actionId, const QString &message, const QString &iconName, const Details &details, const QString &cookie,
        const Identity::List &identities, AsyncResult *result){

    if (identities.isEmpty()){
        result->setError("No identities to authenticate as");
        return;
    }
    Request request{actionId, message, iconName, cookie, details, identities, result};
    if (result_) pending_.enqueue(request);
    else begin(request);
}

void polkitagent::begin(const Request &request){
    cookie_ = request.cookie;
    result_ = request.result;
    identities_ = request.identities;

    // Preselect the current user, else the first identity
    int selected = 0;
    const QString self = UnixUserIdentity(getuid()).toString();
    for (int i = 0; i < identities_.size(); i++){
        if (identities_[i].toString() == self){selected = i; break;}
    }

    dialog_ = new polkitdialog(request.actionId, request.message, request.iconName, request.details, identities_, selected);
    connect(dialog_, &polkitdialog::submitted, this, &polkitagent::passwordSubmitted);
    connect(dialog_, &polkitdialog::identityChanged, this, &polkitagent::identitySelected);
    connect(dialog_, &polkitdialog::rejected, this, &polkitagent::dialogCancelled);
    dialog_->show();
    dialog_->raise();
    dialog_->activateWindow();

    startSession(selected);
}

bool polkitagent::initiateAuthenticationFinish(){
    return true;
}

void polkitagent::cancelAuthentication(){
    // polkit-qt doesn't say which request was cancelled; only act when it can't be a queued one
    if (result_ && pending_.isEmpty()) finish();
}

void polkitagent::startSession(int identityIndex){
    dropSession();
    session_ = new Session(identities_[identityIndex], cookie_, result_, this);
    connect(session_, &Session::request, this, &polkitagent::sessionRequest);
    connect(session_, &Session::completed, this, &polkitagent::sessionCompleted);
    connect(session_, &Session::showError, this, [this](const QString &text){if (dialog_) dialog_->setError(text);});
    connect(session_, &Session::showInfo, this, [this](const QString &text){if (dialog_) dialog_->setInfo(text);});
    session_->initiate();
}

void polkitagent::dropSession(){
    if (!session_) return;
    Session *s = session_;
    session_ = nullptr;
    s->disconnect(this);
    s->cancel();
    s->deleteLater();
}

void polkitagent::sessionRequest(const QString &prompt, bool echo){
    if (dialog_) dialog_->setPrompt(prompt, echo);
}

void polkitagent::sessionCompleted(bool gainedAuthorization){
    if (gainedAuthorization){
        finish();
        return;
    }
    // Wrong password: new PAM conversation for the same identity, waits for user input
    if (dialog_){
        dialog_->setError(tr("Authentication failed. Please try again."));
        startSession(dialog_->identityIndex());
    }
    else finish();
}

void polkitagent::identitySelected(int index){
    if (result_ && index >= 0 && index < identities_.size()) startSession(index);
}

void polkitagent::passwordSubmitted(const QString &password){
    if (session_) session_->setResponse(password);
}

void polkitagent::dialogCancelled(){
    if (result_) finish();
}

void polkitagent::finish(){
    if (cancelling_) return;
    cancelling_ = true;
    dropSession();
    if (dialog_){
        dialog_->disconnect(this);  // close() would otherwise emit rejected
        dialog_->close();
        dialog_->deleteLater();
    }
    AsyncResult *result = result_;
    result_ = nullptr;
    cookie_.clear();
    identities_.clear();
    cancelling_ = false;
    if (result) result->setCompleted();
    if (!pending_.isEmpty()) begin(pending_.dequeue());
}
