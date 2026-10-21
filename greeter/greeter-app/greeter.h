// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GREETER_H
#define GREETER_H

#include <QObject>
#include <QStackedWidget>

#include "greetdclient.h"
#include "userlistmodel.h"
#include "sessionlistmodel.h"
#include "userselectview.h"
#include "passwordview.h"
#include "sessionselectview.h"

namespace loginui { class ScreenSet; }

// greetd login flow, with the card on one of a layer-shell window per screen.
class Greeter : public QObject {
    Q_OBJECT
public:
    explicit Greeter(QObject *parent = nullptr);

private slots:
    void onAuthMessage(const QString &type, const QString &message);
    void onAuthSucceeded();
    void onAuthFailed(const QString &description);
    void onUserSelected(const UserInfo &user);
    void onOtherUserRequested();
    void onLoginAttempted(const QString &username, const QString &password);
    void onBackClicked();
    void onSessionButtonClicked();
    void onSessionSelected(int index);

private:
    QWidget *createCard();
    void initStartupView();
    void restoreLastSession();
    void showUserSelectView();
    void showPasswordView();
    void showSessionSelectView();
    void beginAuth(const QString &username);

    GreetdClient *m_client;
    UserListModel m_users;
    SessionListModel m_sessions;

    QStackedWidget *m_stack;
    UserSelectView *m_userSelectView;
    PasswordView *m_passwordView;
    SessionSelectView *m_sessionSelectView;
    loginui::ScreenSet *m_screens;

    bool m_sessionActive = false;
    bool m_isManualEntry = false;
    bool m_autoSubmitPending = false;
    QString m_pendingPassword;
    QString m_currentUsername;
};

#endif // GREETER_H
