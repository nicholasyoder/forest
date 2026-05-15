// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GREETERWINDOW_H
#define GREETERWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QImage>

#include "greetdclient.h"
#include "userlistmodel.h"
#include "sessionlistmodel.h"

class GreeterWindow : public QWidget {
    Q_OBJECT
public:
    explicit GreeterWindow(QWidget *parent = nullptr);
    ~GreeterWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAuthMessage(const QString &type, const QString &message);
    void onAuthSucceeded();
    void onAuthFailed(const QString &description);
    void onUserChanged(int index);
    void onLoginClicked();
    void onClockTick();

private:
    void setupUi();
    void loadWallpaper();
    void setStatus(const QString &text, bool isError = false);
    void beginAuth();

    GreetdClient *m_client;
    UserListModel m_users;
    SessionListModel m_sessions;

    QComboBox *m_userCombo;
    QLineEdit *m_passwordEdit;
    QComboBox *m_sessionCombo;
    QPushButton *m_loginButton;
    QLabel *m_statusLabel;
    QLabel *m_clockLabel;
    QLabel *m_promptLabel;

    QTimer *m_clockTimer;
    QImage *m_wallpaper = nullptr;
};

#endif // GREETERWINDOW_H
