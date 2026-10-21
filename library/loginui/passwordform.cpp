// SPDX-License-Identifier: LGPL-3.0-or-later

#include "passwordform.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <QWindow>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include "loginui.h"

namespace loginui {

namespace {

// xkb's Lock modifier is always bit 1.
constexpr uint32_t CapsLockMask = 0x2;

// Qt has no public Caps Lock query; QtWayland tracks wl_keyboard.modifiers, sent with every enter.
bool capsLockOn(QWidget *widget)
{
    QWindow *window = widget->window()->windowHandle();
    auto *waylandWindow = window ? dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle()) : nullptr;
    auto *device = waylandWindow ? waylandWindow->display()->defaultInputDevice() : nullptr;
    auto *keyboard = device ? device->keyboard() : nullptr;
    return keyboard && (keyboard->mNativeModifiers & CapsLockMask);
}

} // namespace

PasswordForm::PasswordForm(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_header = new QWidget;
    auto *headerLayout = new QVBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    m_avatar = new QLabel;
    m_avatar->setObjectName("login_PasswordAvatar");
    m_avatar->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_avatar, 0, Qt::AlignHCenter);
    m_nameLabel = new QLabel;
    m_nameLabel->setObjectName("login_UserDisplayName");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(m_nameLabel);
    layout->addWidget(m_header);

    m_input = new QLineEdit;
    m_input->setObjectName("login_PasswordEdit");
    m_input->setEchoMode(QLineEdit::Password);
    m_input->setPlaceholderText("Password");
    // Lock and layer surfaces can't place popups.
    m_input->setContextMenuPolicy(Qt::NoContextMenu);
    m_input->installEventFilter(this);
    layout->addWidget(m_input);

    m_capsLockLabel = new QLabel("Caps Lock is on");
    m_capsLockLabel->setObjectName("login_CapsLockLabel");
    m_capsLockLabel->setAlignment(Qt::AlignCenter);
    m_capsLockLabel->hide();
    layout->addWidget(m_capsLockLabel);

    m_extraRows = new QVBoxLayout;
    m_extraRows->setSpacing(8);
    layout->addLayout(m_extraRows);

    m_buttonRow = new QHBoxLayout;
    m_buttonRow->addStretch();
    m_submitButton = new QPushButton("Unlock");
    m_submitButton->setObjectName("login_LoginButton");
    m_submitButton->setDefault(true);
    m_buttonRow->addWidget(m_submitButton);
    layout->addLayout(m_buttonRow);

    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("login_StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    connect(m_submitButton, &QPushButton::clicked, this, &PasswordForm::submit);
    connect(m_input, &QLineEdit::returnPressed, this, &PasswordForm::submit);

    setBusy();
}

void PasswordForm::setUser(const QString &displayName, const QString &avatarPath)
{
    m_avatar->setPixmap(circularAvatar(avatarPath, 96, displayName));
    m_nameLabel->setText(displayName);
}

void PasswordForm::setHeaderVisible(bool visible)
{
    m_header->setVisible(visible);
}

void PasswordForm::setButtonText(const QString &text)
{
    m_submitButton->setText(text);
}

void PasswordForm::addRow(QLayout *row)
{
    m_extraRows->addLayout(row);
}

void PasswordForm::addLeadingButton(QPushButton *button)
{
    m_buttonRow->insertWidget(m_buttonRow->count() - 2, button);
}

void PasswordForm::setPrompt(const QString &prompt, bool secret)
{
    QString text = prompt.trimmed();
    if (text.endsWith(':'))
        text.chop(1);
    m_input->setPlaceholderText(text.isEmpty() ? (secret ? "Password" : "Input") : text);
    m_input->setEchoMode(secret ? QLineEdit::Password : QLineEdit::Normal);
    m_input->setEnabled(true);
    m_submitButton->setEnabled(true);
    m_input->setFocus();
}

void PasswordForm::focusInput()
{
    if (m_input->isEnabled())
        m_input->setFocus();
}

void PasswordForm::setBusy()
{
    m_input->clear();
    m_input->setEnabled(false);
    m_submitButton->setEnabled(false);
}

void PasswordForm::setStatus(const QString &text, bool error)
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

bool PasswordForm::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        m_capsLockLabel->setVisible(capsLockOn(m_input));
        return QWidget::eventFilter(watched, event);
    }
    // QtWayland's nativeModifiers include locked mods. Caps Lock's own press/release carry the
    // pre-toggle state (unlock lands after release), so predict the toggle from its press and
    // ignore its release.
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return QWidget::eventFilter(watched, event);
    auto *key = static_cast<QKeyEvent *>(event);
    const bool locked = key->nativeModifiers() & CapsLockMask;
    if (key->key() != Qt::Key_CapsLock)
        m_capsLockLabel->setVisible(locked);
    else if (event->type() == QEvent::KeyPress && !key->isAutoRepeat())
        m_capsLockLabel->setVisible(!locked);
    return QWidget::eventFilter(watched, event);
}

void PasswordForm::submit()
{
    if (!m_input->isEnabled())
        return;
    const QString response = m_input->text();
    setStatus({}, false);
    setBusy();
    emit submitted(response);
}

} // namespace loginui
