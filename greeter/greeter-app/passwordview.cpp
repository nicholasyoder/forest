// SPDX-License-Identifier: LGPL-3.0-or-later

#include "passwordview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>

#include "loginui.h"

PasswordView::PasswordView(const QList<SessionInfo> &sessions, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 0, 0, 0);

    // --- Header stack ---
    m_headerStack = new QStackedWidget;

    // Index 0: known user
    auto *knownHeader = new QWidget;
    auto *knownLay = new QVBoxLayout(knownHeader);
    knownLay->setContentsMargins(0, 0, 0, 0);
    knownLay->setSpacing(6);
    knownLay->setAlignment(Qt::AlignHCenter);
    m_avatarLabel = new QLabel;
    m_avatarLabel->setObjectName("greeter_PasswordAvatar");
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    knownLay->addWidget(m_avatarLabel, 0, Qt::AlignHCenter);
    m_displayNameLabel = new QLabel;
    m_displayNameLabel->setObjectName("greeter_UserDisplayName");
    m_displayNameLabel->setAlignment(Qt::AlignCenter);
    knownLay->addWidget(m_displayNameLabel);
    m_headerStack->addWidget(knownHeader);

    // Index 1: manual entry
    auto *manualHeader = new QWidget;
    auto *manualLay = new QVBoxLayout(manualHeader);
    manualLay->setContentsMargins(0, 0, 0, 0);
    manualLay->setSpacing(6);
    manualLay->setAlignment(Qt::AlignHCenter);
    m_genericAvatarLabel = new QLabel;
    m_genericAvatarLabel->setObjectName("greeter_PasswordAvatar");
    m_genericAvatarLabel->setPixmap(loginui::circularAvatar({}, 96, "?"));
    m_genericAvatarLabel->setAlignment(Qt::AlignCenter);
    manualLay->addWidget(m_genericAvatarLabel, 0, Qt::AlignHCenter);
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setObjectName("greeter_UsernameEdit");
    m_usernameEdit->setPlaceholderText("Username");
    manualLay->addWidget(m_usernameEdit);
    m_headerStack->addWidget(manualHeader);

    layout->addWidget(m_headerStack);

    // --- Password field ---
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setObjectName("greeter_PasswordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Password");
    layout->addWidget(m_passwordEdit);

    // --- Session row ---
    auto *sessionRow = new QHBoxLayout;
    auto *sessionLabel = new QLabel("Session:");
    sessionLabel->setObjectName("greeter_Label");
    m_sessionBtn = new QPushButton(sessions.isEmpty() ? QString() : sessions.first().name);
    m_sessionBtn->setObjectName("greeter_SessionButton");
    sessionRow->addWidget(sessionLabel);
    sessionRow->addWidget(m_sessionBtn, 1);
    layout->addLayout(sessionRow);

    // --- Button row ---
    auto *btnRow = new QHBoxLayout;
    m_backButton = new QPushButton("Back");
    m_backButton->setObjectName("greeter_BackButton");
    m_loginButton = new QPushButton("Login");
    m_loginButton->setObjectName("greeter_LoginButton");
    m_loginButton->setDefault(true);
    btnRow->addWidget(m_backButton);
    btnRow->addStretch();
    btnRow->addWidget(m_loginButton);
    layout->addLayout(btnRow);

    // --- Status label ---
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("greeter_StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    connect(m_loginButton, &QPushButton::clicked, this, &PasswordView::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &PasswordView::onLoginClicked);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, [this]() {
        m_passwordEdit->setFocus();
    });
    connect(m_backButton, &QPushButton::clicked, this, &PasswordView::backClicked);
    connect(m_sessionBtn, &QPushButton::clicked, this, &PasswordView::sessionButtonClicked);
}

void PasswordView::setUser(const UserInfo &user)
{
    m_isManualMode = false;
    m_knownUsername = user.username;
    m_avatarLabel->setPixmap(loginui::circularAvatar(user.faceIconPath, 96, user.displayName));
    m_displayNameLabel->setText(user.displayName);
    m_headerStack->setCurrentIndex(0);
}

void PasswordView::setManualEntry(const QString &prefillUsername)
{
    m_isManualMode = true;
    m_knownUsername.clear();
    m_usernameEdit->setText(prefillUsername);
    m_headerStack->setCurrentIndex(1);
}

void PasswordView::setPrompt(const QString &prompt, bool secret)
{
    m_passwordEdit->setPlaceholderText(prompt.isEmpty() ? (secret ? "Password" : "Input") : prompt);
    m_passwordEdit->setEchoMode(secret ? QLineEdit::Password : QLineEdit::Normal);
    m_passwordEdit->setFocus();
}

void PasswordView::setStatus(const QString &text, bool isError)
{
    if (text.isEmpty()) {
        m_statusLabel->hide();
        return;
    }
    m_statusLabel->setText(text);
    m_statusLabel->setProperty("error", isError);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    m_statusLabel->show();
}

void PasswordView::clearPassword()
{
    m_passwordEdit->clear();
}

void PasswordView::focusInput()
{
    if (m_isManualMode && m_usernameEdit->text().isEmpty())
        m_usernameEdit->setFocus();
    else
        m_passwordEdit->setFocus();
}

void PasswordView::setLoginEnabled(bool enabled)
{
    m_loginButton->setEnabled(enabled);
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

void PasswordView::onLoginClicked()
{
    emit loginAttempted(getUsername(), m_passwordEdit->text());
}
