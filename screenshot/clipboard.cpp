// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clipboard.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QTimer>
#include <QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandinputdevice_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "qwayland-ext-data-control-v1.h"
#include "qwayland-wlr-data-control-unstable-v1.h"

#include <poll.h>

namespace {

constexpr int kReadyFd = 3;

void writeAll(int fd, const QByteArray &data) {
    // Blocking write: a reader that stalls only stalls this helper.
    const char *p = data.constData();
    qsizetype left = data.size();
    while (left > 0) {
        ssize_t n = write(fd, p, size_t(left));
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        p += n;
        left -= n;
    }
}

class ExtManager : public QWaylandClientExtensionTemplate<ExtManager>,
                   public QtWayland::ext_data_control_manager_v1 {
public:
    ExtManager() : QWaylandClientExtensionTemplate<ExtManager>(1) {}
};

class WlrManager : public QWaylandClientExtensionTemplate<WlrManager>,
                   public QtWayland::zwlr_data_control_manager_v1 {
public:
    WlrManager() : QWaylandClientExtensionTemplate<WlrManager>(1) {}
};

class ExtSource : public QtWayland::ext_data_control_source_v1 {
public:
    ExtSource(::ext_data_control_source_v1 *object, const QByteArray &png)
        : QtWayland::ext_data_control_source_v1(object), m_png(png) {}
    ~ExtSource() override { destroy(); }

protected:
    void ext_data_control_source_v1_send(const QString &, int32_t fd) override {
        writeAll(fd, m_png);
        close(fd);
    }
    void ext_data_control_source_v1_cancelled() override { QCoreApplication::quit(); }

private:
    QByteArray m_png;
};

class WlrSource : public QtWayland::zwlr_data_control_source_v1 {
public:
    WlrSource(::zwlr_data_control_source_v1 *object, const QByteArray &png)
        : QtWayland::zwlr_data_control_source_v1(object), m_png(png) {}
    ~WlrSource() override { destroy(); }

protected:
    void zwlr_data_control_source_v1_send(const QString &, int32_t fd) override {
        writeAll(fd, m_png);
        close(fd);
    }
    void zwlr_data_control_source_v1_cancelled() override { QCoreApplication::quit(); }

private:
    QByteArray m_png;
};

template <class Device, class Source, class Manager>
void publish(Manager &manager, wl_seat *seat, const QByteArray &png) {
    auto *device = new Device(manager.get_data_device(seat));
    auto *source = new Source(manager.create_data_source(), png);
    source->offer(QStringLiteral("image/png"));
    device->set_selection(source->object());
}

QByteArray readAll(int fd) {
    QByteArray data;
    char buf[65536];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        data.append(buf, n);
    }
    return data;
}

} // namespace

bool clipboard::copyPng(const QByteArray &png) {
    const QByteArray self = QCoreApplication::applicationFilePath().toLocal8Bit();
    int fds[2], ready[2];
    if (pipe2(fds, O_CLOEXEC) < 0)
        return false;
    if (pipe2(ready, O_CLOEXEC) < 0) {
        close(fds[0]);
        close(fds[1]);
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        close(ready[0]);
        close(ready[1]);
        return false;
    }
    if (pid == 0) {
        // Child: only async-signal-safe calls until exec.
        setsid();
        dup2(fds[0], STDIN_FILENO);
        if (ready[1] == kReadyFd)
            fcntl(kReadyFd, F_SETFD, 0);
        else
            dup2(ready[1], kReadyFd); // dup2 clears CLOEXEC
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
        }
        execl(self.constData(), self.constData(), "--serve-clipboard", nullptr);
        _exit(127);
    }

    close(fds[0]);
    close(ready[1]);
    signal(SIGPIPE, SIG_IGN); // helper may die early
    const char *p = png.constData();
    qsizetype left = png.size();
    bool ok = true;
    while (left > 0) {
        ssize_t n = write(fds[1], p, size_t(left));
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0) {
            ok = false;
            break;
        }
        p += n;
        left -= n;
    }
    close(fds[1]);

    // Copied only once the helper owns the selection; it exits without a byte on failure.
    if (ok) {
        pollfd pfd{ready[0], POLLIN, 0};
        char c = 0;
        ok = poll(&pfd, 1, 3000) == 1 && read(ready[0], &c, 1) == 1;
    }
    close(ready[0]);
    return ok;
}

int clipboard::serve() {
    const QByteArray png = readAll(STDIN_FILENO);
    if (png.isEmpty())
        return 1;

    int argc = 1;
    char name[] = "forest-screenshot";
    char *argv[] = {name, nullptr};
    QGuiApplication app(argc, argv);

    ExtManager ext;
    WlrManager wlr;
    int rc = 1;
    // The managers bind on the first event-loop pass.
    QTimer::singleShot(0, &app, [&] {
        auto *display = QtWaylandClient::QWaylandIntegration::instance()->display();
        auto *seat = display ? display->defaultInputDevice() : nullptr;
        if (!seat || !(ext.isActive() || wlr.isActive())) {
            QCoreApplication::exit(1);
            return;
        }
        if (ext.isActive())
            publish<QtWayland::ext_data_control_device_v1, ExtSource>(ext, seat->wl_seat(), png);
        else
            publish<QtWayland::zwlr_data_control_device_v1, WlrSource>(wlr, seat->wl_seat(), png);
        display->forceRoundTrip(); // the compositor has seen the selection (or errored)
        // Tell the parent we own the clipboard.
        if (write(kReadyFd, "K", 1) == 1)
            rc = 0;
        close(kReadyFd);
    });
    // Returns when another client takes the selection (`cancelled`).
    int exitCode = app.exec();
    return exitCode ? exitCode : rc;
}
