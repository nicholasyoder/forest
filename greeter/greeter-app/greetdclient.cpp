// SPDX-License-Identifier: LGPL-3.0-or-later

#include "greetdclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>
#include <QDebug>

GreetdClient::GreetdClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
{
    connect(m_socket, &QLocalSocket::readyRead, this, &GreetdClient::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &GreetdClient::onSocketError);

    QString socketPath = QProcessEnvironment::systemEnvironment().value("GREETD_SOCK");
    if (socketPath.isEmpty()) {
        qWarning() << "GREETD_SOCK environment variable not set";
        return;
    }
    m_socket->connectToServer(socketPath);
}

void GreetdClient::createSession(const QString &username) {
    QJsonObject msg;
    msg["type"] = "create_session";
    msg["username"] = username;
    sendMessage(msg);
}

void GreetdClient::postAuthResponse(const QString &response) {
    QJsonObject msg;
    msg["type"] = "post_auth_message_response";
    if (response.isNull())
        msg["response"] = QJsonValue(QJsonValue::Null);
    else
        msg["response"] = response;
    sendMessage(msg);
}

void GreetdClient::startSession(const QString &sessionExec) {
    QJsonArray cmd;
    for (const QString &part : sessionExec.split(' ', Qt::SkipEmptyParts))
        cmd.append(part);
    QJsonObject msg;
    msg["type"] = "start_session";
    msg["cmd"] = cmd;
    msg["env"] = QJsonArray();
    sendMessage(msg);
}

void GreetdClient::cancelSession() {
    QJsonObject msg;
    msg["type"] = "cancel_session";
    sendMessage(msg);
}

void GreetdClient::sendMessage(const QJsonObject &msg) {
    QByteArray json = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    uint32_t length = static_cast<uint32_t>(json.size());

    QByteArray packet;
    packet.resize(4);
    packet[0] = static_cast<char>(length & 0xFF);
    packet[1] = static_cast<char>((length >> 8) & 0xFF);
    packet[2] = static_cast<char>((length >> 16) & 0xFF);
    packet[3] = static_cast<char>((length >> 24) & 0xFF);
    packet.append(json);

    m_socket->write(packet);
    m_socket->flush();
}

void GreetdClient::onReadyRead() {
    m_buffer.append(m_socket->readAll());

    while (true) {
        if (m_buffer.size() < 4)
            break;

        uint32_t length = static_cast<uint8_t>(m_buffer[0])
                        | (static_cast<uint32_t>(static_cast<uint8_t>(m_buffer[1])) << 8)
                        | (static_cast<uint32_t>(static_cast<uint8_t>(m_buffer[2])) << 16)
                        | (static_cast<uint32_t>(static_cast<uint8_t>(m_buffer[3])) << 24);

        if (m_buffer.size() < 4 + static_cast<int>(length))
            break;

        QByteArray json = m_buffer.mid(4, static_cast<int>(length));
        m_buffer.remove(0, 4 + static_cast<int>(length));

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json, &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "greetd JSON parse error:" << err.errorString();
            continue;
        }
        processMessage(doc.object());
    }
}

void GreetdClient::processMessage(const QJsonObject &msg) {
    QString type = msg["type"].toString();

    if (type == "success") {
        emit authSucceeded();
    } else if (type == "error") {
        emit authFailed(msg["description"].toString());
    } else if (type == "auth_message") {
        emit authMessage(msg["auth_message_type"].toString(), msg["auth_message"].toString());
    } else {
        qWarning() << "Unknown greetd message type:" << type;
    }
}

void GreetdClient::onSocketError(QLocalSocket::LocalSocketError error) {
    qWarning() << "greetd socket error:" << error << m_socket->errorString();
}
