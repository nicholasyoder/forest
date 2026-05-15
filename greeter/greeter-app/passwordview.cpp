// SPDX-License-Identifier: LGPL-3.0-or-later

#include "passwordview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QPainter>
#include <QPainterPath>

static QPixmap makeCircularPixmap(const QString &path, int size, const QString &fallback)
{
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);

    QPixmap src;
    if (!path.isEmpty())
        src.load(path);

    if (!src.isNull()) {
        src = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPainterPath clip;
        clip.addEllipse(0, 0, size, size);
        p.setClipPath(clip);
        p.drawPixmap((size - src.width()) / 2, (size - src.height()) / 2, src);
    } else {
        p.setBrush(QColor(90, 90, 90));
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, size, size);
        p.setPen(Qt::white);
        QFont f;
        f.setPixelSize(size / 2);
        f.setBold(true);
        p.setFont(f);
        QString ch = fallback.isEmpty() ? "?" : fallback.left(1).toUpper();
        p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, ch);
    }
    return result;
}

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
    m_genericAvatarLabel->setPixmap(makeCircularPixmap({}, 96, "?"));
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
    m_sessionCombo = new QComboBox;
    m_sessionCombo->setObjectName("greeter_SessionCombo");
    for (const SessionInfo &s : sessions)
        m_sessionCombo->addItem(s.name);
    sessionRow->addWidget(sessionLabel);
    sessionRow->addWidget(m_sessionCombo, 1);
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
}

void PasswordView::setUser(const UserInfo &user)
{
    m_isManualMode = false;
    m_knownUsername = user.username;
    m_avatarLabel->setPixmap(makeCircularPixmap(user.faceIconPath, 96, user.displayName));
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
    return m_sessionCombo->currentIndex();
}

void PasswordView::onLoginClicked()
{
    emit loginAttempted(getUsername(), m_passwordEdit->text());
}
