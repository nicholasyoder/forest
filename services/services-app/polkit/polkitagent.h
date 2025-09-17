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

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);
    void showError(const QString &text);
    void showInfo(const QString &text);

private slots:
    void initiate_session(PolkitQt1::Identity session_identity);

private:
    bool auth_in_progress = false;
    bool auth_canceled = false;
    QString auth_cookie;
    PolkitQt1::Agent::AsyncResult *auth_result = nullptr;
    QHash<PolkitQt1::Agent::Session*, PolkitQt1::Identity> session_identities;
    polkitdialog * pkwidget = nullptr;
};

#endif // POLKITAGENT_H
