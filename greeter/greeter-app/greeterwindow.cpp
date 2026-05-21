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
#include <QPushButton>
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
    connect(m_client, &GreetdClient::sessionStarted, this, [this]() {
        QSettings settings("Forest", "Forest");
        settings.setValue("greeter/last_user", m_currentUsername);
        int idx = m_passwordView->sessionIndex();
        const auto &sessions = m_sessions.sessions();
        if (idx >= 0 && idx < sessions.size())
            settings.setValue("greeter/last_session", sessions.at(idx).exec);
        settings.sync();
        QApplication::quit();
    });

    connect(m_clockTimer, &QTimer::timeout, this, &GreeterWindow::onClockTick);
    m_clockTimer->start(1000);

    setupUi();
    loadWallpaper();
    initStartupView();
}

GreeterWindow::~GreeterWindow()
{
    delete m_wallpaper;
}

void GreeterWindow::setupUi()
{
    // Confine layout content to the primary screen's area within the (possibly
    // multi-monitor) fullscreen surface that cage spans across all outputs.
    QRect primary = QGuiApplication::primaryScreen()->geometry();
    QRect virt    = QGuiApplication::primaryScreen()->virtualGeometry();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(
        primary.left()   - virt.left(),
        primary.top()    - virt.top(),
        virt.right()     - primary.right(),
        virt.bottom()    - primary.bottom()
    );
    mainLayout->addStretch(1);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch(1);

    auto *card = new QFrame;
    card->setObjectName("greeter_Card");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);

    // View stack
    m_stack = new QStackedWidget;
    m_userSelectView = new UserSelectView(m_users.users());
    m_passwordView = new PasswordView(m_sessions.sessions());
    m_sessionSelectView = new SessionSelectView(m_sessions.sessions());
    m_stack->addWidget(m_userSelectView);    // index 0
    m_stack->addWidget(m_passwordView);      // index 1
    m_stack->addWidget(m_sessionSelectView); // index 2
    cardLayout->addWidget(m_stack);

    // Power buttons — always visible below the stack
    auto *powerRow = new QHBoxLayout;
    powerRow->addStretch();
    auto *shutdownBtn = new QPushButton("Shutdown");
    shutdownBtn->setObjectName("greeter_PowerButton");
    auto *rebootBtn = new QPushButton("Reboot");
    rebootBtn->setObjectName("greeter_PowerButton");
    powerRow->addWidget(shutdownBtn);
    powerRow->addWidget(rebootBtn);
    cardLayout->addLayout(powerRow);

    centerRow->addWidget(card);
    centerRow->addStretch(1);

    auto *clock_layout = new QVBoxLayout;
    m_clockLabel = new QLabel;
    m_clockLabel->setObjectName("greeter_Clock");
    m_clockLabel->setAlignment(Qt::AlignCenter);
    onClockTick();
    clock_layout->addStretch(1);
    clock_layout->addWidget(m_clockLabel);
    clock_layout->addStretch(1);

    centerRow->addLayout(clock_layout);
    centerRow->addStretch(1);


    mainLayout->addLayout(centerRow);
    mainLayout->addStretch(1);

    connect(m_userSelectView, &UserSelectView::userSelected,
            this, &GreeterWindow::onUserSelected);
    connect(m_userSelectView, &UserSelectView::otherUserRequested,
            this, &GreeterWindow::onOtherUserRequested);
    connect(m_passwordView, &PasswordView::loginAttempted,
            this, &GreeterWindow::onLoginAttempted);
    connect(m_passwordView, &PasswordView::backClicked,
            this, &GreeterWindow::onBackClicked);
    connect(m_passwordView, &PasswordView::sessionButtonClicked,
            this, &GreeterWindow::onSessionButtonClicked);
    connect(m_sessionSelectView, &SessionSelectView::sessionSelected,
            this, &GreeterWindow::onSessionSelected);
    connect(m_sessionSelectView, &SessionSelectView::cancelled,
            this, &GreeterWindow::showPasswordView);

    connect(shutdownBtn, &QPushButton::clicked, this, []() {
        QProcess::startDetached("systemctl", {"poweroff"});
    });
    connect(rebootBtn, &QPushButton::clicked, this, []() {
        QProcess::startDetached("systemctl", {"reboot"});
    });
}

void GreeterWindow::loadWallpaper()
{
    QSettings settings("Forest", "Forest");
    QString wallpaperFile = settings.value("wallpaper/file", "/usr/share/wallpapers/forest/forest.jpg").toString();
    if (wallpaperFile.isEmpty())
        return;
    QSize screenSize = QGuiApplication::primaryScreen()->geometry().size();
    m_wallpaper = miscutills::get_wallpaper_scaled(wallpaperFile, Fill, screenSize);
}

void GreeterWindow::restoreLastSession()
{
    QString lastExec = QSettings("Forest", "Forest").value("greeter/last_session").toString();
    if (lastExec.isEmpty())
        return;
    const auto &sessions = m_sessions.sessions();
    for (int i = 0; i < sessions.size(); ++i) {
        if (sessions[i].exec == lastExec) {
            m_passwordView->setSelectedSession(i, sessions[i].name);
            return;
        }
    }
}

void GreeterWindow::initStartupView()
{
    QSettings settings("Forest", "Forest");
    QString lastUser = settings.value("greeter/last_user").toString();

    if (!lastUser.isEmpty()) {
        for (const UserInfo &user : m_users.users()) {
            if (user.username == lastUser) {
                m_userSelectView->highlightUser(lastUser);
                m_passwordView->setUser(user);
                restoreLastSession();
                showPasswordView();
                beginAuth(lastUser);
                return;
            }
        }
        // Last user not in the list (e.g. root) — open manual entry pre-filled
        m_isManualEntry = true;
        m_passwordView->setManualEntry(lastUser);
        restoreLastSession();
        showPasswordView();
        m_passwordView->focusInput();
        return;
    }

    if (!m_users.users().isEmpty()) {
        showUserSelectView();
    } else {
        // No enumerable users at all — open manual entry
        m_isManualEntry = true;
        m_passwordView->setManualEntry();
        showPasswordView();
        m_passwordView->focusInput();
    }
}

void GreeterWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPoint origin = QGuiApplication::primaryScreen()->virtualGeometry().topLeft();
    for (QScreen *screen : QGuiApplication::screens()) {
        QRect r = screen->geometry().translated(-origin);
        if (m_wallpaper && !m_wallpaper->isNull())
            painter.drawImage(r, *m_wallpaper, m_wallpaper->rect());
        else
            painter.fillRect(r, QColor(30, 30, 30));
    }
}

void GreeterWindow::onClockTick()
{
    m_clockLabel->setText(QDateTime::currentDateTime().toString("hh:mm"));
}

void GreeterWindow::showUserSelectView()
{
    m_stack->setCurrentIndex(0);
}

void GreeterWindow::showPasswordView()
{
    m_stack->setCurrentIndex(1);
}

void GreeterWindow::showSessionSelectView()
{
    m_stack->setCurrentIndex(2);
}

void GreeterWindow::onSessionButtonClicked()
{
    m_sessionSelectView->highlightSession(m_passwordView->sessionIndex());
    showSessionSelectView();
}

void GreeterWindow::onSessionSelected(int index)
{
    const auto &sessions = m_sessions.sessions();
    if (index >= 0 && index < sessions.size())
        m_passwordView->setSelectedSession(index, sessions[index].name);
    showPasswordView();
}

void GreeterWindow::beginAuth(const QString &username)
{
    m_currentUsername = username;
    m_sessionActive = true;
    m_client->createSession(username);
}

void GreeterWindow::onUserSelected(const UserInfo &user)
{
    if (m_sessionActive)
        m_client->cancelSession();

    m_sessionActive = false;
    m_isManualEntry = false;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setUser(user);
    m_passwordView->clearPassword();
    m_passwordView->setStatus({});
    showPasswordView();
    beginAuth(user.username);
}

void GreeterWindow::onOtherUserRequested()
{
    if (m_sessionActive)
        m_client->cancelSession();

    m_sessionActive = false;
    m_isManualEntry = true;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setManualEntry();
    m_passwordView->clearPassword();
    m_passwordView->setStatus({});
    showPasswordView();
    m_passwordView->focusInput();
}

void GreeterWindow::onLoginAttempted(const QString &username, const QString &password)
{
    if (m_isManualEntry && !m_sessionActive) {
        // First login click in manual mode: start the session then auto-submit the password
        // when greetd sends the first secret auth_message.
        m_pendingPassword = password;
        m_autoSubmitPending = true;
        m_passwordView->setLoginEnabled(false);
        beginAuth(username);
    } else {
        // Known-user mode (session already started by onUserSelected) or a subsequent
        // PAM round after the initial auto-submit.
        m_passwordView->setLoginEnabled(false);
        m_client->postAuthResponse(password);
    }
}

void GreeterWindow::onBackClicked()
{
    if (m_sessionActive)
        m_client->cancelSession();

    m_sessionActive = false;
    m_isManualEntry = false;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setStatus({});
    showUserSelectView();
}

void GreeterWindow::onAuthMessage(const QString &type, const QString &message)
{
    if (m_autoSubmitPending && type == "secret") {
        // Silently submit the pre-filled password — user never sees this exchange.
        QString pwd = m_pendingPassword;
        m_pendingPassword.clear();
        m_autoSubmitPending = false;
        m_client->postAuthResponse(pwd);
        return;
    }

    // If PAM asked for something unexpected before the secret prompt, abandon auto-submit.
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    if (type == "secret") {
        m_passwordView->setPrompt(message, true);
        m_passwordView->setLoginEnabled(true);
    } else if (type == "visible") {
        m_passwordView->setPrompt(message, false);
        m_passwordView->setLoginEnabled(true);
    } else if (type == "info") {
        m_passwordView->setStatus(message, false);
        m_client->postAuthResponse({});
    } else if (type == "error") {
        m_passwordView->setStatus(message, true);
        m_client->postAuthResponse({});
    }
}

void GreeterWindow::onAuthSucceeded()
{
    int idx = m_passwordView->sessionIndex();
    if (idx < 0 || idx >= m_sessions.sessions().size()) {
        qWarning() << "No session selected";
        return;
    }
    m_client->startSession(m_sessions.sessions().at(idx).exec);
}

void GreeterWindow::onAuthFailed(const QString &description)
{
    m_sessionActive = false;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setStatus(
        description.isEmpty() ? "Authentication failed" : description, true);
    m_passwordView->clearPassword();
    m_passwordView->setLoginEnabled(true);

    if (!m_isManualEntry) {
        // Auto-restart the session so the user can try again immediately.
        beginAuth(m_currentUsername);
    } else {
        // Manual entry: wait for the next Login click before creating a new session.
        m_passwordView->focusInput();
    }
}
