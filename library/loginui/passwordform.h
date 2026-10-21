// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PASSWORDFORM_H
#define PASSWORDFORM_H

#include <QWidget>

class QBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

namespace loginui {

// Avatar + name header, an auth prompt and a submit button. Starts busy; setPrompt() enables it.
class PasswordForm : public QWidget {
    Q_OBJECT
public:
    explicit PasswordForm(QWidget *parent = nullptr);

    void setUser(const QString &displayName, const QString &avatarPath);
    void setHeaderVisible(bool visible);
    void setButtonText(const QString &text);
    // Below the prompt, above the button row.
    void addRow(QLayout *row);
    // Left of the submit button.
    void addLeadingButton(QPushButton *button);

    void setPrompt(const QString &prompt, bool secret);
    // Clears and disables the input until the next setPrompt().
    void setBusy();
    void focusInput();
    void setStatus(const QString &text, bool error);

signals:
    void submitted(const QString &response);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void submit();

    QWidget *m_header;
    QLabel *m_avatar;
    QLabel *m_nameLabel;
    QLineEdit *m_input;
    QLabel *m_capsLockLabel;
    QBoxLayout *m_extraRows;
    QHBoxLayout *m_buttonRow;
    QPushButton *m_submitButton;
    QLabel *m_statusLabel;
};

} // namespace loginui

#endif // PASSWORDFORM_H
