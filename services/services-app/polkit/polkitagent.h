// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef POLKITAGENT_H
#define POLKITAGENT_H

#include <QPointer>
#include <QQueue>

#include <polkit-qt6-1/polkitqt1-agent-session.h>
#include <polkit-qt6-1/polkitqt1-identity.h>
#include <polkit-qt6-1/polkitqt1-details.h>
#include <polkit-qt6-1/polkitqt1-agent-listener.h>
#include <polkit-qt6-1/polkitqt1-subject.h>

#include "polkitdialog.h"

class polkitagent : public PolkitQt1::Agent::Listener{
    Q_OBJECT

public:
    polkitagent(QObject *parent = nullptr);
    ~polkitagent();

public slots:
    void initiateAuthentication(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details,
                                const QString &cookie, const PolkitQt1::Identity::List &identities, PolkitQt1::Agent::AsyncResult *result);

    bool initiateAuthenticationFinish();
    void cancelAuthentication();

private slots:
    void sessionRequest(const QString &prompt, bool echo);
    void sessionCompleted(bool gainedAuthorization);
    void identitySelected(int index);
    void passwordSubmitted(const QString &password);
    void dialogCancelled();

private:
    struct Request{
        QString actionId, message, iconName, cookie;
        PolkitQt1::Details details;
        PolkitQt1::Identity::List identities;
        PolkitQt1::Agent::AsyncResult *result = nullptr;
    };

    void begin(const Request &request);
    void startSession(int identityIndex);
    void dropSession();
    void finish();  // Tears down the active request and completes its result

    // State of the single active request; result == nullptr means idle.
    QQueue<Request> pending_;  // Requests waiting for the active one
    QString cookie_;
    PolkitQt1::Agent::AsyncResult *result_ = nullptr;
    PolkitQt1::Identity::List identities_;
    PolkitQt1::Agent::Session *session_ = nullptr;
    QPointer<polkitdialog> dialog_;
    bool cancelling_ = false;
};

#endif // POLKITAGENT_H
