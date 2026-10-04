// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PAMAUTH_H
#define PAMAUTH_H

#include <QByteArray>
#include <QMutex>
#include <QObject>
#include <QSemaphore>
#include <QThread>

#include <atomic>

struct pam_message;
struct pam_response;

// One pam_authenticate() round for the current user, run off the GUI thread.
// PAM drives the prompts; answer each prompt() with respond().
class PamAuth : public QObject {
    Q_OBJECT
public:
    explicit PamAuth(QObject *parent = nullptr);
    ~PamAuth() override;

    void start();
    void respond(const QString &response);
    bool isRunning() const { return m_thread && m_thread->isRunning(); }
    // Whether the current or last round asked for input.
    bool prompted() const { return m_prompted; }

signals:
    void prompt(const QString &message, bool secret);
    void message(const QString &text, bool error);
    void succeeded();
    void failed(const QString &reason);

private:
    void run();
    void finish();
    static int converse(int count, const pam_message **msgs, pam_response **resps, void *data);

    QThread *m_thread = nullptr;
    QSemaphore m_responseReady;
    QMutex m_mutex;
    QByteArray m_response;
    std::atomic<bool> m_cancelled{false};
    std::atomic<bool> m_awaitingResponse{false};
    std::atomic<bool> m_prompted{false};
    int m_result = 0;
    QString m_reason;
};

#endif // PAMAUTH_H
