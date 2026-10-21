// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PASSWORDVIEW_H
#define PASSWORDVIEW_H

#include <QWidget>
#include <QList>
#include "userlistmodel.h"
#include "sessionlistmodel.h"

class QLineEdit;
class QPushButton;
namespace loginui { class PasswordForm; }

// loginui::PasswordForm plus manual username entry, the session row and Back.
class PasswordView : public QWidget {
    Q_OBJECT
public:
    explicit PasswordView(const QList<SessionInfo> &sessions, QWidget *parent = nullptr);

    void setUser(const UserInfo &user);
    void setManualEntry(const QString &prefillUsername = {});

    void setPrompt(const QString &prompt, bool secret);
    void setStatus(const QString &text, bool isError = false);
    // Clears the input. Known user: busy until greetd prompts. Manual entry: ready, since the
    // greetd session only starts on submit.
    void resetPrompt();
    void focusInput();

    QString getUsername() const;
    int sessionIndex() const;
    void setSelectedSession(int index, const QString &name);

signals:
    void backClicked();
    void loginAttempted(const QString &username, const QString &password);
    void sessionButtonClicked();

private:
    QWidget *m_manualHeader;
    QLineEdit *m_usernameEdit;
    loginui::PasswordForm *m_form;
    QPushButton *m_sessionBtn;
    int m_sessionIndex = 0;

    bool m_isManualMode = false;
    QString m_knownUsername;
};

#endif // PASSWORDVIEW_H
