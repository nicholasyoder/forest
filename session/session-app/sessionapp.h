// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONAPP_H
#define SESSIONAPP_H

#include <QObject>
#include <QSettings>

class SessionApp : public QObject
{
    Q_OBJECT

public:
    SessionApp();
    ~SessionApp();

    void startSession();

private slots:
    void launch_autostart_commands();
    void launch_autostart_xdg();

private:
    void startProcess(QString cmd);

    QSettings *settings = nullptr;

};
#endif // SESSIONAPP_H
