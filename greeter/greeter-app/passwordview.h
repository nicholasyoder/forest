// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PASSWORDVIEW_H
#define PASSWORDVIEW_H

#include <QWidget>
#include <QList>
#include "userlistmodel.h"
#include "sessionlistmodel.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;

class PasswordView : public QWidget {
    Q_OBJECT
public:
    explicit PasswordView(const QList<SessionInfo> &sessions, QWidget *parent = nullptr);

    void setUser(const UserInfo &user);
    void setManualEntry(const QString &prefillUsername = {});

    void setPrompt(const QString &prompt, bool secret);
    void setStatus(const QString &text, bool isError = false);
    void clearPassword();
    void focusInput();
    void setLoginEnabled(bool enabled);

    QString getUsername() const;
    int sessionIndex() const;
    void setSelectedSession(int index, const QString &name);

signals:
    void backClicked();
    void loginAttempted(const QString &username, const QString &password);
    void sessionButtonClicked();

private slots:
    void onLoginClicked();

private:
    QStackedWidget *m_headerStack;
    // Header index 0: known user (avatar + display name)
    QLabel *m_avatarLabel;
    QLabel *m_displayNameLabel;
    // Header index 1: manual entry (generic avatar + username field)
    QLabel *m_genericAvatarLabel;
    QLineEdit *m_usernameEdit;

    QLineEdit *m_passwordEdit;
    QPushButton *m_sessionBtn;
    int m_sessionIndex = 0;
    QPushButton *m_loginButton;
    QPushButton *m_backButton;
    QLabel *m_statusLabel;

    bool m_isManualMode = false;
    QString m_knownUsername;
};

#endif // PASSWORDVIEW_H
