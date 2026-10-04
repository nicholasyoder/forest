// SPDX-License-Identifier: LGPL-3.0-or-later

#include "passwordcard.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include <pwd.h>
#include <unistd.h>

#include "loginui.h"

PasswordCard::PasswordCard(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("greeter_Card");

    QString username, displayName, homeDir;
    if (passwd *pw = getpwuid(getuid())) {
        username = QString::fromLocal8Bit(pw->pw_name);
        homeDir = QString::fromLocal8Bit(pw->pw_dir);
        displayName = QString::fromLocal8Bit(pw->pw_gecos).split(',').first().trimmed();
    }
    if (displayName.isEmpty())
        displayName = username;

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);

    auto *avatar = new QLabel;
    avatar->setObjectName("greeter_PasswordAvatar");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setPixmap(loginui::circularAvatar(loginui::faceIconPath(username, homeDir), 96, displayName));
    layout->addWidget(avatar, 0, Qt::AlignHCenter);

    auto *nameLabel = new QLabel(displayName);
    nameLabel->setObjectName("greeter_UserDisplayName");
    nameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(nameLabel);

    m_input = new QLineEdit;
    m_input->setObjectName("greeter_PasswordEdit");
    m_input->setEchoMode(QLineEdit::Password);
    m_input->setPlaceholderText("Password");
    // A lock surface can't parent popups.
    m_input->setContextMenuPolicy(Qt::NoContextMenu);
    m_input->installEventFilter(this);
    layout->addWidget(m_input);

    m_capsLockLabel = new QLabel("Caps Lock is on");
    m_capsLockLabel->setObjectName("greeter_CapsLockLabel");
    m_capsLockLabel->setAlignment(Qt::AlignCenter);
    m_capsLockLabel->hide();
    layout->addWidget(m_capsLockLabel);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_unlockButton = new QPushButton("Unlock");
    m_unlockButton->setObjectName("greeter_LoginButton");
    m_unlockButton->setDefault(true);
    btnRow->addWidget(m_unlockButton);
    layout->addLayout(btnRow);

    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("greeter_StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    connect(m_unlockButton, &QPushButton::clicked, this, &PasswordCard::submit);
    connect(m_input, &QLineEdit::returnPressed, this, &PasswordCard::submit);

    setBusy();
}

void PasswordCard::setPrompt(const QString &prompt, bool secret)
{
    QString text = prompt.trimmed();
    if (text.endsWith(':'))
        text.chop(1);
    m_input->setPlaceholderText(text.isEmpty() ? (secret ? "Password" : "Input") : text);
    m_input->setEchoMode(secret ? QLineEdit::Password : QLineEdit::Normal);
    m_input->setEnabled(true);
    m_unlockButton->setEnabled(true);
    m_input->setFocus();
}

void PasswordCard::setBusy()
{
    m_input->setEnabled(false);
    m_unlockButton->setEnabled(false);
}

void PasswordCard::setStatus(const QString &text, bool error)
{
    if (text.isEmpty()) {
        m_statusLabel->hide();
        return;
    }
    m_statusLabel->setText(text);
    m_statusLabel->setProperty("error", error);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->show();
}

bool PasswordCard::eventFilter(QObject *watched, QEvent *event)
{
    // QtWayland's nativeModifiers include locked mods; xkb's Lock is always bit 1.
    // Caps Lock's own press/release carry the pre-toggle state (unlock lands after release),
    // so predict the toggle from its press and ignore its release.
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return QFrame::eventFilter(watched, event);
    auto *key = static_cast<QKeyEvent *>(event);
    const bool locked = key->nativeModifiers() & 0x2;
    if (key->key() != Qt::Key_CapsLock)
        m_capsLockLabel->setVisible(locked);
    else if (event->type() == QEvent::KeyPress && !key->isAutoRepeat())
        m_capsLockLabel->setVisible(!locked);
    return QFrame::eventFilter(watched, event);
}

void PasswordCard::submit()
{
    if (!m_input->isEnabled())
        return;
    const QString response = m_input->text();
    m_input->clear();
    setStatus({}, false);
    setBusy();
    emit submitted(response);
}
