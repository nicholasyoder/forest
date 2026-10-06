// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <QWaylandClientExtension>

#include <functional>

#include "outputtypes.h"
#include "qwayland-wlr-output-management-unstable-v1.h"

class OutputHead;

// wlr-output-management client. state() is only updated on the manager's
// `done`, so it's always a consistent snapshot.
class OutputManager : public QWaylandClientExtensionTemplate<OutputManager>, public QtWayland::zwlr_output_manager_v1{
    Q_OBJECT

public:
    enum Result { Succeeded, Failed, Cancelled };
    using Callback = std::function<void(Result)>;

    explicit OutputManager(QObject *parent = nullptr);
    ~OutputManager();

    // False until the first `done`.
    bool isReady() const{return m_ready;}
    const OutputState &state() const{return m_state;}

    // `layout` must list every head to enable; heads it omits are disabled.
    // A stale serial is retried once against the next state.
    void apply(const OutputLayout &layout, const Callback &callback);
    void test(const OutputLayout &layout, const Callback &callback);

    void removeHead(OutputHead *head);

signals:
    void stateChanged();

protected:
    void zwlr_output_manager_v1_head(struct ::zwlr_output_head_v1 *head) override;
    void zwlr_output_manager_v1_done(uint32_t serial) override;
    void zwlr_output_manager_v1_finished() override;

private:
    struct Request {
        OutputLayout layout;
        Callback callback;
        bool test = false;
        bool retried = false;
    };
    void submit(const Request &request);

    QList<OutputHead *> m_heads;
    OutputState m_state;
    uint32_t m_serial = 0;
    bool m_ready = false;
    QList<Request> m_waitingForDone; // cancelled before the newer state arrived
};

#endif // OUTPUTMANAGER_H
