// SPDX-License-Identifier: LGPL-3.0-or-later

#include "greeterwindow.h"

#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDateTime>
#include <QProcess>
#include <QSettings>
#include <QDebug>

#include "miscutills.h"

GreeterWindow::GreeterWindow(QWidget *parent)
    : QWidget(parent)
    , m_client(new GreetdClient(this))
    , m_clockTimer(new QTimer(this))
{
    setWindowFlags(Qt::FramelessWindowHint);

    connect(m_client, &GreetdClient::authMessage, this, &GreeterWindow::onAuthMessage);
    connect(m_client, &GreetdClient::authSucceeded, this, &GreeterWindow::onAuthSucceeded);
    connect(m_client, &GreetdClient::authFailed, this, &GreeterWindow::onAuthFailed);

    connect(m_clockTimer, &QTimer::timeout, this, &GreeterWindow::onClockTick);
    m_clockTimer->start(1000);

    setupUi();
    loadWallpaper();

    if (!m_users.users().isEmpty())
        beginAuth();
}

GreeterWindow::~GreeterWindow() {
    delete m_wallpaper;
}

void GreeterWindow::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Center the card vertically and horizontally
    mainLayout->addStretch();
    QHBoxLayout *centerRow = new QHBoxLayout;
    centerRow->addStretch();

    QFrame *card = new QFrame;
    card->setObjectName("greeter_Card");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);

    // Clock
    m_clockLabel = new QLabel;
    m_clockLabel->setObjectName("greeter_Clock");
    m_clockLabel->setAlignment(Qt::AlignCenter);
    onClockTick();
    cardLayout->addWidget(m_clockLabel);

    // User selector
    QLabel *userLabel = new QLabel("User:");
    userLabel->setObjectName("greeter_Label");
    cardLayout->addWidget(userLabel);
    m_userCombo = new QComboBox;
    m_userCombo->setObjectName("greeter_UserCombo");
    for (const UserInfo &user : m_users.users())
        m_userCombo->addItem(user.displayName, user.username);
    cardLayout->addWidget(m_userCombo);

    // Prompt label (shows PAM prompt text, e.g. "Password:")
    m_promptLabel = new QLabel("Password:");
    m_promptLabel->setObjectName("greeter_PromptLabel");
    cardLayout->addWidget(m_promptLabel);

    // Password / input field
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setObjectName("greeter_PasswordEdit");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter password...");
    cardLayout->addWidget(m_passwordEdit);

    // Session selector
    QLabel *sessionLabel = new QLabel("Session:");
    sessionLabel->setObjectName("greeter_Label");
    cardLayout->addWidget(sessionLabel);
    m_sessionCombo = new QComboBox;
    m_sessionCombo->setObjectName("greeter_SessionCombo");
    for (const SessionInfo &session : m_sessions.sessions())
        m_sessionCombo->addItem(session.name);
    cardLayout->addWidget(m_sessionCombo);

    // Login button
    m_loginButton = new QPushButton("Login");
    m_loginButton->setObjectName("greeter_LoginButton");
    m_loginButton->setDefault(true);
    cardLayout->addWidget(m_loginButton);

    // Status label (hidden initially)
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("greeter_StatusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    cardLayout->addWidget(m_statusLabel);

    // Power buttons row
    QHBoxLayout *powerRow = new QHBoxLayout;
    powerRow->addStretch();
    QPushButton *shutdownBtn = new QPushButton("Shutdown");
    shutdownBtn->setObjectName("greeter_PowerButton");
    QPushButton *rebootBtn = new QPushButton("Reboot");
    rebootBtn->setObjectName("greeter_PowerButton");
    powerRow->addWidget(shutdownBtn);
    powerRow->addWidget(rebootBtn);
    cardLayout->addLayout(powerRow);

    centerRow->addWidget(card);
    centerRow->addStretch();
    mainLayout->addLayout(centerRow);
    mainLayout->addStretch();

    connect(m_userCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GreeterWindow::onUserChanged);
    connect(m_loginButton, &QPushButton::clicked, this, &GreeterWindow::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &GreeterWindow::onLoginClicked);
    connect(shutdownBtn, &QPushButton::clicked, this, [](){
        QProcess::startDetached("systemctl", {"poweroff"});
    });
    connect(rebootBtn, &QPushButton::clicked, this, [](){
        QProcess::startDetached("systemctl", {"reboot"});
    });
}

void GreeterWindow::loadWallpaper() {
    QSettings settings("Forest", "Forest");
    QString wallpaperFile = settings.value("wallpaper/file").toString();
    if (wallpaperFile.isEmpty())
        return;

    QSize screenSize = QGuiApplication::primaryScreen()->geometry().size();
    m_wallpaper = miscutills::get_wallpaper_scaled(wallpaperFile, Fill, screenSize);
}

void GreeterWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    if (m_wallpaper && !m_wallpaper->isNull()) {
        painter.drawImage(rect(), *m_wallpaper, m_wallpaper->rect());
    } else {
        painter.fillRect(rect(), QColor(30, 30, 30));
    }
}

void GreeterWindow::onClockTick() {
    m_clockLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
}

void GreeterWindow::onUserChanged(int /*index*/) {
    m_client->cancelSession();
    m_passwordEdit->clear();
    setStatus(QString());
    beginAuth();
}

void GreeterWindow::onLoginClicked() {
    QString password = m_passwordEdit->text();
    m_loginButton->setEnabled(false);
    m_client->postAuthResponse(password);
}

void GreeterWindow::onAuthMessage(const QString &type, const QString &message) {
    if (type == "secret") {
        m_promptLabel->setText(message.isEmpty() ? "Password:" : message);
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setFocus();
        m_loginButton->setEnabled(true);
    } else if (type == "visible") {
        m_promptLabel->setText(message.isEmpty() ? "Input:" : message);
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_passwordEdit->setFocus();
        m_loginButton->setEnabled(true);
    } else if (type == "info") {
        setStatus(message, false);
        m_client->postAuthResponse(QString()); // null response to continue
    } else if (type == "error") {
        setStatus(message, true);
        m_client->postAuthResponse(QString()); // null response to continue
    }
}

void GreeterWindow::onAuthSucceeded() {
    int sessionIndex = m_sessionCombo->currentIndex();
    if (sessionIndex < 0 || sessionIndex >= m_sessions.sessions().size()) {
        qWarning() << "No session selected";
        return;
    }
    const QString &exec = m_sessions.sessions().at(sessionIndex).exec;
    m_client->startSession(exec);
}

void GreeterWindow::onAuthFailed(const QString &description) {
    setStatus(description.isEmpty() ? "Authentication failed" : description, true);
    m_passwordEdit->clear();
    m_passwordEdit->setFocus();
    m_loginButton->setEnabled(true);
    // Restart auth for the current user
    beginAuth();
}

void GreeterWindow::setStatus(const QString &text, bool isError) {
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

void GreeterWindow::beginAuth() {
    QString username = m_userCombo->currentData().toString();
    if (username.isEmpty())
        return;
    m_client->createSession(username);
}
