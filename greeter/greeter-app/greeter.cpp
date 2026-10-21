// SPDX-License-Identifier: LGPL-3.0-or-later

#include "greeter.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QProcess>
#include <QSettings>
#include <QPushButton>
#include <QDebug>
#include <QWindow>

#include <LayerShellQt/Window>

#include "loginui.h"
#include "screenset.h"

Greeter::Greeter(QObject *parent)
    : QObject(parent)
    , m_client(new GreetdClient(this))
{
    connect(m_client, &GreetdClient::authMessage, this, &Greeter::onAuthMessage);
    connect(m_client, &GreetdClient::authSucceeded, this, &Greeter::onAuthSucceeded);
    connect(m_client, &GreetdClient::authFailed, this, &Greeter::onAuthFailed);
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

    // Biome focuses the newest exclusive surface, so primary goes last.
    m_screens = new loginui::ScreenSet(createCard(), [](QWindow *window) {
        auto *layer = LayerShellQt::Window::get(window);
        layer->setLayer(LayerShellQt::Window::LayerOverlay);
        layer->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                                                        | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
        layer->setExclusiveZone(-1);
        layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
        layer->setScope("forest-greeter");
    }, loginui::ScreenSet::InitialFocus::Last, this);
    connect(m_screens, &loginui::ScreenSet::cardPlaced, this, [this] {
        if (m_stack->currentWidget() == m_passwordView)
            m_passwordView->focusInput();
    });

    initStartupView();
    m_screens->showOnAllScreens();
}

QWidget *Greeter::createCard()
{
    auto *card = new loginui::Card;

    // View stack
    m_stack = new QStackedWidget;
    m_userSelectView = new UserSelectView(m_users.users());
    m_passwordView = new PasswordView(m_sessions.sessions());
    m_sessionSelectView = new SessionSelectView(m_sessions.sessions());
    m_stack->addWidget(m_userSelectView);    // index 0
    m_stack->addWidget(m_passwordView);      // index 1
    m_stack->addWidget(m_sessionSelectView); // index 2
    card->addWidget(m_stack);

    // Power buttons — always visible below the stack
    auto *powerRow = new QHBoxLayout;
    powerRow->addStretch();
    auto *shutdownBtn = new QPushButton("Shutdown");
    shutdownBtn->setObjectName("login_PowerButton");
    auto *rebootBtn = new QPushButton("Reboot");
    rebootBtn->setObjectName("login_PowerButton");
    powerRow->addWidget(shutdownBtn);
    powerRow->addWidget(rebootBtn);
    card->addLayout(powerRow);

    connect(m_userSelectView, &UserSelectView::userSelected,
            this, &Greeter::onUserSelected);
    connect(m_userSelectView, &UserSelectView::otherUserRequested,
            this, &Greeter::onOtherUserRequested);
    connect(m_passwordView, &PasswordView::loginAttempted,
            this, &Greeter::onLoginAttempted);
    connect(m_passwordView, &PasswordView::backClicked,
            this, &Greeter::onBackClicked);
    connect(m_passwordView, &PasswordView::sessionButtonClicked,
            this, &Greeter::onSessionButtonClicked);
    connect(m_sessionSelectView, &SessionSelectView::sessionSelected,
            this, &Greeter::onSessionSelected);
    connect(m_sessionSelectView, &SessionSelectView::cancelled,
            this, &Greeter::showPasswordView);

    connect(shutdownBtn, &QPushButton::clicked, this, []() {
        QProcess::startDetached("systemctl", {"poweroff"});
    });
    connect(rebootBtn, &QPushButton::clicked, this, []() {
        QProcess::startDetached("systemctl", {"reboot"});
    });
    return card;
}

void Greeter::restoreLastSession()
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

void Greeter::initStartupView()
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

void Greeter::showUserSelectView()
{
    m_stack->setCurrentIndex(0);
}

void Greeter::showPasswordView()
{
    m_stack->setCurrentIndex(1);
}

void Greeter::showSessionSelectView()
{
    m_stack->setCurrentIndex(2);
}

void Greeter::onSessionButtonClicked()
{
    m_sessionSelectView->highlightSession(m_passwordView->sessionIndex());
    showSessionSelectView();
}

void Greeter::onSessionSelected(int index)
{
    const auto &sessions = m_sessions.sessions();
    if (index >= 0 && index < sessions.size())
        m_passwordView->setSelectedSession(index, sessions[index].name);
    showPasswordView();
}

void Greeter::beginAuth(const QString &username)
{
    m_currentUsername = username;
    m_sessionActive = true;
    m_client->createSession(username);
}

void Greeter::onUserSelected(const UserInfo &user)
{
    if (m_sessionActive)
        m_client->cancelSession();

    m_sessionActive = false;
    m_isManualEntry = false;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setUser(user);
    m_passwordView->setStatus({});
    showPasswordView();
    beginAuth(user.username);
}

void Greeter::onOtherUserRequested()
{
    if (m_sessionActive)
        m_client->cancelSession();

    m_sessionActive = false;
    m_isManualEntry = true;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setManualEntry();
    m_passwordView->setStatus({});
    showPasswordView();
    m_passwordView->focusInput();
}

void Greeter::onLoginAttempted(const QString &username, const QString &password)
{
    if (m_isManualEntry && !m_sessionActive) {
        // First login click in manual mode: start the session then auto-submit the password
        // when greetd sends the first secret auth_message.
        m_pendingPassword = password;
        m_autoSubmitPending = true;
        beginAuth(username);
    } else {
        // Known-user mode (session already started by onUserSelected) or a subsequent
        // PAM round after the initial auto-submit.
        m_client->postAuthResponse(password);
    }
}

void Greeter::onBackClicked()
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

void Greeter::onAuthMessage(const QString &type, const QString &message)
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
    } else if (type == "visible") {
        m_passwordView->setPrompt(message, false);
    } else if (type == "info") {
        m_passwordView->setStatus(message, false);
        m_client->postAuthResponse({});
    } else if (type == "error") {
        m_passwordView->setStatus(message, true);
        m_client->postAuthResponse({});
    }
}

void Greeter::onAuthSucceeded()
{
    int idx = m_passwordView->sessionIndex();
    if (idx < 0 || idx >= m_sessions.sessions().size()) {
        qWarning() << "No session selected";
        return;
    }
    const SessionInfo &session = m_sessions.sessions().at(idx);
    QStringList cmd = QProcess::splitCommand(session.exec);
    if (session.type == SessionType::X11)
        cmd.prepend("/usr/share/forest/forest-greeter-xsession");

    // What other display managers derive from the .desktop file; greetd also
    // hands XDG_SESSION_TYPE to logind.
    QStringList env;
    env << QStringLiteral("XDG_SESSION_TYPE=%1").arg(session.type == SessionType::X11 ? "x11" : "wayland");
    env << QStringLiteral("XDG_SESSION_DESKTOP=%1").arg(session.desktopId);
    if (!session.desktopNames.isEmpty())
        env << QStringLiteral("XDG_CURRENT_DESKTOP=%1").arg(session.desktopNames);
    m_client->startSession(cmd, env);
}

void Greeter::onAuthFailed(const QString &description)
{
    m_sessionActive = false;
    m_autoSubmitPending = false;
    m_pendingPassword.clear();

    m_passwordView->setStatus(
        description.isEmpty() ? "Authentication failed" : description, true);
    m_passwordView->resetPrompt();

    if (!m_isManualEntry) {
        // Auto-restart the session so the user can try again immediately.
        beginAuth(m_currentUsername);
    } else {
        // Manual entry: wait for the next Login click before creating a new session.
        m_passwordView->focusInput();
    }
}
