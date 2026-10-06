// SPDX-License-Identifier: LGPL-3.0-or-later

#include "outputmanager.h"

#include <QDebug>

#include "wayland-wlr-output-management-unstable-v1-client-protocol.h"

namespace {

uint32_t proxyVersion(void *object){
    return wl_proxy_get_version(static_cast<wl_proxy *>(object));
}

}

class OutputMode : public QtWayland::zwlr_output_mode_v1{
public:
    OutputMode(struct ::zwlr_output_mode_v1 *object, OutputHead *head)
        : QtWayland::zwlr_output_mode_v1(object), m_head(head){}
    ~OutputMode(){
        // `release` is v3+; older objects can only be dropped client-side.
        if (proxyVersion(object()) >= ZWLR_OUTPUT_MODE_V1_RELEASE_SINCE_VERSION)
            release();
        else
            wl_proxy_destroy(reinterpret_cast<wl_proxy *>(object()));
    }

    OutputModeInfo info;

protected:
    void zwlr_output_mode_v1_size(int32_t width, int32_t height) override{info.size = QSize(width, height);}
    void zwlr_output_mode_v1_refresh(int32_t refresh) override{info.refresh = refresh;}
    void zwlr_output_mode_v1_preferred() override{info.preferred = true;}
    void zwlr_output_mode_v1_finished() override;

private:
    OutputHead *m_head;
};

class OutputHead : public QtWayland::zwlr_output_head_v1{
public:
    OutputHead(struct ::zwlr_output_head_v1 *object, OutputManager *manager)
        : QtWayland::zwlr_output_head_v1(object), m_manager(manager){}
    ~OutputHead(){
        qDeleteAll(m_modes);
        if (proxyVersion(object()) >= ZWLR_OUTPUT_HEAD_V1_RELEASE_SINCE_VERSION)
            release();
        else
            wl_proxy_destroy(reinterpret_cast<wl_proxy *>(object()));
    }

    OutputHeadInfo snapshot() const{
        OutputHeadInfo result = m_info;
        for (OutputMode *mode : m_modes){
            if (mode == m_current && m_info.enabled) result.currentMode = result.modes.size();
            result.modes << mode->info;
        }
        return result;
    }

    // Exact size + refresh match, or nullptr.
    OutputMode *findMode(QSize size, int refresh) const{
        for (OutputMode *mode : m_modes)
            if (mode->info.size == size && mode->info.refresh == refresh) return mode;
        return nullptr;
    }

    QString name() const{return m_info.name;}

    void removeMode(OutputMode *mode){
        m_modes.removeOne(mode);
        if (m_current == mode) m_current = nullptr;
        delete mode;
    }

protected:
    void zwlr_output_head_v1_name(const QString &name) override{m_info.name = name;}
    void zwlr_output_head_v1_description(const QString &description) override{m_info.description = description;}
    void zwlr_output_head_v1_make(const QString &make) override{m_info.make = make;}
    void zwlr_output_head_v1_model(const QString &model) override{m_info.model = model;}
    void zwlr_output_head_v1_serial_number(const QString &serial) override{m_info.serial = serial;}
    void zwlr_output_head_v1_mode(struct ::zwlr_output_mode_v1 *mode) override{m_modes << new OutputMode(mode, this);}
    void zwlr_output_head_v1_enabled(int32_t enabled) override{m_info.enabled = enabled;}
    void zwlr_output_head_v1_current_mode(struct ::zwlr_output_mode_v1 *mode) override{
        m_current = static_cast<OutputMode *>(QtWayland::zwlr_output_mode_v1::fromObject(mode));
    }
    void zwlr_output_head_v1_position(int32_t x, int32_t y) override{m_info.pos = QPoint(x, y);}
    void zwlr_output_head_v1_transform(int32_t transform) override{m_info.transform = transform;}
    void zwlr_output_head_v1_scale(wl_fixed_t scale) override{m_info.scale = wl_fixed_to_double(scale);}
    void zwlr_output_head_v1_adaptive_sync(uint32_t state) override{
        m_info.adaptiveSync = state == ZWLR_OUTPUT_HEAD_V1_ADAPTIVE_SYNC_STATE_ENABLED;
    }
    void zwlr_output_head_v1_finished() override{m_manager->removeHead(this);}

private:
    OutputManager *m_manager;
    OutputHeadInfo m_info; // modes/currentMode filled in by snapshot()
    QList<OutputMode *> m_modes;
    OutputMode *m_current = nullptr;
};

void OutputMode::zwlr_output_mode_v1_finished(){
    m_head->removeMode(this);
}

class OutputConfiguration : public QObject, public QtWayland::zwlr_output_configuration_v1{
    Q_OBJECT

public:
    OutputConfiguration(struct ::zwlr_output_configuration_v1 *object, uint32_t serial)
        : QtWayland::zwlr_output_configuration_v1(object), serial(serial){}
    ~OutputConfiguration(){
        // Configuration heads die with the configuration; only the proxies remain.
        for (struct ::zwlr_output_configuration_head_v1 *head : heads)
            wl_proxy_destroy(reinterpret_cast<wl_proxy *>(head));
        destroy();
    }

    const uint32_t serial;
    QList<struct ::zwlr_output_configuration_head_v1 *> heads;

signals:
    void finished(OutputManager::Result result);

protected:
    void zwlr_output_configuration_v1_succeeded() override{done(OutputManager::Succeeded);}
    void zwlr_output_configuration_v1_failed() override{done(OutputManager::Failed);}
    void zwlr_output_configuration_v1_cancelled() override{done(OutputManager::Cancelled);}

private:
    void done(OutputManager::Result result){
        emit finished(result);
        deleteLater();
    }
};

OutputManager::OutputManager(QObject *parent)
    : QWaylandClientExtensionTemplate<OutputManager>(4){
    setParent(parent);
}

OutputManager::~OutputManager(){
    qDeleteAll(m_heads);
}

void OutputManager::apply(const OutputLayout &layout, const Callback &callback){
    submit({layout, callback, false, false});
}

void OutputManager::test(const OutputLayout &layout, const Callback &callback){
    submit({layout, callback, true, false});
}

void OutputManager::submit(const Request &request){
    if (!isActive() || !m_ready){
        qWarning() << "OutputManager: no output state yet, can't" << (request.test ? "test" : "apply");
        if (request.callback) request.callback(Failed);
        return;
    }

    auto *config = new OutputConfiguration(create_configuration(m_serial), m_serial);
    for (OutputHead *head : std::as_const(m_heads)){
        auto it = std::find_if(request.layout.begin(), request.layout.end(),
                               [&](const OutputConfig &c){ return c.connector == head->name(); });
        if (it == request.layout.end() || !it->enabled){
            config->disable_head(head->object());
            continue;
        }

        struct ::zwlr_output_configuration_head_v1 *ch = config->enable_head(head->object());
        config->heads << ch;
        // Always send a mode: disabled heads have no current one to fall back on.
        if (OutputMode *mode = head->findMode(it->size, it->refresh))
            zwlr_output_configuration_head_v1_set_mode(ch, mode->object());
        else
            zwlr_output_configuration_head_v1_set_custom_mode(ch, it->size.width(), it->size.height(), it->refresh);
        zwlr_output_configuration_head_v1_set_position(ch, it->pos.x(), it->pos.y());
        zwlr_output_configuration_head_v1_set_transform(ch, it->transform);
        zwlr_output_configuration_head_v1_set_scale(ch, wl_fixed_from_double(it->scale));
        if (m_state.adaptiveSyncSupported)
            zwlr_output_configuration_head_v1_set_adaptive_sync(ch, it->adaptiveSync
                ? ZWLR_OUTPUT_HEAD_V1_ADAPTIVE_SYNC_STATE_ENABLED
                : ZWLR_OUTPUT_HEAD_V1_ADAPTIVE_SYNC_STATE_DISABLED);
    }

    connect(config, &OutputConfiguration::finished, this, [this, request, serial = config->serial](Result result){
        if (result == Cancelled && !request.retried){
            Request retry = request;
            retry.retried = true;
            // The newer state's `done` normally arrives before `cancelled`.
            if (m_serial != serial) submit(retry);
            else m_waitingForDone << retry;
            return;
        }
        if (result != Succeeded)
            qWarning() << "OutputManager:" << (request.test ? "test" : "apply")
                       << (result == Failed ? "failed" : "cancelled");
        if (request.callback) request.callback(result);
    });

    if (request.test) config->test();
    else config->apply();
}

void OutputManager::removeHead(OutputHead *head){
    m_heads.removeOne(head);
    delete head;
}

void OutputManager::zwlr_output_manager_v1_head(struct ::zwlr_output_head_v1 *head){
    m_heads << new OutputHead(head, this);
}

void OutputManager::zwlr_output_manager_v1_done(uint32_t serial){
    m_serial = serial;
    m_state.heads.clear();
    for (OutputHead *head : std::as_const(m_heads))
        m_state.heads << head->snapshot();
    m_state.adaptiveSyncSupported = proxyVersion(object()) >= ZWLR_OUTPUT_HEAD_V1_ADAPTIVE_SYNC_SINCE_VERSION;
    m_ready = true;
    emit stateChanged();

    const QList<Request> waiting = std::exchange(m_waitingForDone, {});
    for (const Request &request : waiting)
        submit(request);
}

void OutputManager::zwlr_output_manager_v1_finished(){
    // No destructor request; the proxy is destroyed client-side only.
    qWarning() << "OutputManager: compositor finished the output manager";
    qDeleteAll(m_heads);
    m_heads.clear();
    m_ready = false;
    wl_proxy_destroy(reinterpret_cast<wl_proxy *>(object()));
}

#include "outputmanager.moc"
