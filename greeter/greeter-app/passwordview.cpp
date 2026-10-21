// SPDX-License-Identifier: LGPL-3.0-or-later

#include "passwordview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "loginui.h"
#include "passwordform.h"

PasswordView::PasswordView(const QList<SessionInfo> &sessions, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 0, 0, 0);

    // Replaces the form's header in manual mode.
    m_manualHeader = new QWidget;
    auto *manualLay = new QVBoxLayout(m_manualHeader);
    manualLay->setContentsMargins(0, 0, 0, 0);
    manualLay->setSpacing(8);
    auto *genericAvatar = new QLabel;
    genericAvatar->setObjectName("greeter_PasswordAvatar");
    genericAvatar->setPixmap(loginui::circularAvatar({}, 96, "?"));
    genericAvatar->setAlignment(Qt::AlignCenter);
    manualLay->addWidget(genericAvatar, 0, Qt::AlignHCenter);
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setObjectName("greeter_UsernameEdit");
    m_usernameEdit->setPlaceholderText("Username");
    manualLay->addWidget(m_usernameEdit);
    m_manualHeader->hide();
    layout->addWidget(m_manualHeader);

    m_form = new loginui::PasswordForm;
    m_form->setButtonText("Login");
    layout->addWidget(m_form);

    auto *sessionRow = new QHBoxLayout;
    auto *sessionLabel = new QLabel("Session:");
    sessionLabel->setObjectName("greeter_Label");
    m_sessionBtn = new QPushButton(sessions.isEmpty() ? QString() : sessions.first().name);
    m_sessionBtn->setObjectName("greeter_SessionButton");
    sessionRow->addWidget(sessionLabel);
    sessionRow->addWidget(m_sessionBtn, 1);
    m_form->addRow(sessionRow);

    auto *backButton = new QPushButton("Back");
    backButton->setObjectName("greeter_BackButton");
    m_form->addLeadingButton(backButton);

    connect(m_form, &loginui::PasswordForm::submitted, this, [this](const QString &password) {
        emit loginAttempted(getUsername(), password);
    });
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_form, &loginui::PasswordForm::focusInput);
    connect(backButton, &QPushButton::clicked, this, &PasswordView::backClicked);
    connect(m_sessionBtn, &QPushButton::clicked, this, &PasswordView::sessionButtonClicked);
}

void PasswordView::setUser(const UserInfo &user)
{
    m_isManualMode = false;
    m_knownUsername = user.username;
    m_form->setUser(user.displayName, user.faceIconPath);
    m_manualHeader->hide();
    m_form->setHeaderVisible(true);
    resetPrompt();
}

void PasswordView::setManualEntry(const QString &prefillUsername)
{
    m_isManualMode = true;
    m_knownUsername.clear();
    m_usernameEdit->setText(prefillUsername);
    m_form->setHeaderVisible(false);
    m_manualHeader->show();
    resetPrompt();
}

void PasswordView::setPrompt(const QString &prompt, bool secret)
{
    m_form->setPrompt(prompt, secret);
}

void PasswordView::setStatus(const QString &text, bool isError)
{
    m_form->setStatus(text, isError);
}

void PasswordView::resetPrompt()
{
    m_form->setBusy();
    if (m_isManualMode)
        m_form->setPrompt({}, true);
}

void PasswordView::focusInput()
{
    if (m_isManualMode && m_usernameEdit->text().isEmpty())
        m_usernameEdit->setFocus();
    else
        m_form->focusInput();
}

QString PasswordView::getUsername() const
{
    return m_isManualMode ? m_usernameEdit->text().trimmed() : m_knownUsername;
}

int PasswordView::sessionIndex() const
{
    return m_sessionIndex;
}

void PasswordView::setSelectedSession(int index, const QString &name)
{
    m_sessionIndex = index;
    m_sessionBtn->setText(name);
}
