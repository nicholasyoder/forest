// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GREETDCLIENT_H
#define GREETDCLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QByteArray>
#include <QJsonObject>

class GreetdClient : public QObject {
    Q_OBJECT
public:
    explicit GreetdClient(QObject *parent = nullptr);

public slots:
    void createSession(const QString &username);
    void postAuthResponse(const QString &response);
    void startSession(const QString &sessionExec);
    void cancelSession();

signals:
    void authMessage(const QString &type, const QString &message);
    void authSucceeded();
    void authFailed(const QString &description);

private slots:
    void onReadyRead();
    void onSocketError(QLocalSocket::LocalSocketError error);

private:
    void sendMessage(const QJsonObject &msg);
    void processMessage(const QJsonObject &msg);

    QLocalSocket *m_socket;
    QByteArray m_buffer;
};

#endif // GREETDCLIENT_H
