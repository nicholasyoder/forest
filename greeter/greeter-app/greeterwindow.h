// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GREETERWINDOW_H
#define GREETERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QStackedWidget>

#include "greetdclient.h"
#include "userlistmodel.h"
#include "sessionlistmodel.h"
#include "userselectview.h"
#include "passwordview.h"
#include "sessionselectview.h"
#include "loginui.h"

class GreeterWindow : public QWidget {
    Q_OBJECT
public:
    explicit GreeterWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

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
    void setupUi();
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
    loginui::Wallpaper m_wallpaper;

    bool m_sessionActive = false;
    bool m_isManualEntry = false;
    bool m_autoSubmitPending = false;
    QString m_pendingPassword;
    QString m_currentUsername;
};

#endif // GREETERWINDOW_H
