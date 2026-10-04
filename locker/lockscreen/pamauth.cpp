// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pamauth.h"

#include <QDebug>

#include <cstdlib>
#include <cstring>
#include <pwd.h>
#include <security/pam_appl.h>
#include <unistd.h>

PamAuth::PamAuth(QObject *parent)
    : QObject(parent)
{
}

PamAuth::~PamAuth()
{
    if (!m_thread)
        return;
    m_cancelled = true;
    m_responseReady.release();
    m_thread->wait();
    delete m_thread;
}

void PamAuth::start()
{
    if (isRunning())
        return;
    delete m_thread;
    m_responseReady.acquire(m_responseReady.available());
    m_thread = QThread::create([this] { run(); });
    // Emit from here, not the worker, so a retry in the handler sees the thread stopped.
    connect(m_thread, &QThread::finished, this, &PamAuth::finish);
    m_thread->start();
}

void PamAuth::respond(const QString &response)
{
    if (!m_awaitingResponse.exchange(false))
        return;
    {
        QMutexLocker locker(&m_mutex);
        explicit_bzero(m_response.data(), m_response.size());
        m_response = response.toUtf8();
    }
    m_responseReady.release();
}

void PamAuth::run()
{
    m_result = PAM_AUTH_ERR;
    passwd *pw = getpwuid(getuid());
    if (!pw) {
        m_reason = "Cannot look up the current user";
        return;
    }

    pam_conv conv{&PamAuth::converse, this};
    pam_handle_t *handle = nullptr;
    int ret = pam_start("forest-locker", pw->pw_name, &conv, &handle);
    if (ret != PAM_SUCCESS) {
        m_reason = QString("PAM start failed: %1").arg(pam_strerror(handle, ret));
        return;
    }

    m_result = pam_authenticate(handle, 0);
    m_reason = QString::fromUtf8(pam_strerror(handle, m_result));
    pam_end(handle, m_result);
}

void PamAuth::finish()
{
    if (m_cancelled)
        return;
    if (m_result == PAM_SUCCESS)
        emit succeeded();
    else
        emit failed(m_reason);
}

int PamAuth::converse(int count, const pam_message **msgs, pam_response **resps, void *data)
{
    auto *self = static_cast<PamAuth *>(data);
    // PAM frees the array and each resp string.
    auto *replies = static_cast<pam_response *>(calloc(count, sizeof(pam_response)));
    if (!replies)
        return PAM_BUF_ERR;

    for (int i = 0; i < count; ++i) {
        const QString text = QString::fromUtf8(msgs[i]->msg);
        switch (msgs[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON: {
            self->m_awaitingResponse = true;
            emit self->prompt(text, msgs[i]->msg_style == PAM_PROMPT_ECHO_OFF);
            self->m_responseReady.acquire();
            QMutexLocker locker(&self->m_mutex);
            if (self->m_cancelled) {
                for (int j = 0; j < i; ++j) {
                    if (replies[j].resp) {
                        explicit_bzero(replies[j].resp, strlen(replies[j].resp));
                        free(replies[j].resp);
                    }
                }
                free(replies);
                return PAM_CONV_ERR;
            }
            replies[i].resp = strdup(self->m_response.constData());
            explicit_bzero(self->m_response.data(), self->m_response.size());
            self->m_response.clear();
            break;
        }
        case PAM_ERROR_MSG:
            emit self->message(text, true);
            break;
        case PAM_TEXT_INFO:
            emit self->message(text, false);
            break;
        }
    }

    *resps = replies;
    return PAM_SUCCESS;
}
