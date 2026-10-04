// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PASSWORDCARD_H
#define PASSWORDCARD_H

#include <QFrame>

class QLabel;
class QLineEdit;
class QPushButton;

// The current user's avatar, name and a PAM prompt, styled as the greeter card.
class PasswordCard : public QFrame {
    Q_OBJECT
public:
    explicit PasswordCard(QWidget *parent = nullptr);

    void setPrompt(const QString &prompt, bool secret);
    void setBusy();
    void focusInput();
    void setStatus(const QString &text, bool error);

signals:
    void submitted(const QString &response);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void submit();

    QLineEdit *m_input;
    QPushButton *m_unlockButton;
    QLabel *m_capsLockLabel;
    QLabel *m_statusLabel;
};

#endif // PASSWORDCARD_H
