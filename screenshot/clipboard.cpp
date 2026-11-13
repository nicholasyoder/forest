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

#include "qwayland-wlr-data-control-unstable-v1.h"

namespace {

class Manager : public QWaylandClientExtensionTemplate<Manager>,
                public QtWayland::zwlr_data_control_manager_v1 {
public:
    Manager() : QWaylandClientExtensionTemplate<Manager>(1) {}
};

class Source : public QtWayland::zwlr_data_control_source_v1 {
public:
    Source(::zwlr_data_control_source_v1 *object, const QByteArray &png)
        : QtWayland::zwlr_data_control_source_v1(object), m_png(png) {}
    ~Source() override { destroy(); }

protected:
    void zwlr_data_control_source_v1_send(const QString &, int32_t fd) override {
        // Blocking write: a reader that stalls only stalls this helper.
        const char *p = m_png.constData();
        qsizetype left = m_png.size();
        while (left > 0) {
            ssize_t n = write(fd, p, size_t(left));
            if (n < 0 && errno == EINTR)
                continue;
            if (n <= 0)
                break;
            p += n;
            left -= n;
        }
        close(fd);
    }
    void zwlr_data_control_source_v1_cancelled() override { QCoreApplication::quit(); }

private:
    QByteArray m_png;
};

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
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) < 0)
        return false;

    pid_t pid = fork();
    if (pid < 0) {
        close(fds[0]);
        close(fds[1]);
        return false;
    }
    if (pid == 0) {
        // Child: only async-signal-safe calls until exec.
        setsid();
        dup2(fds[0], STDIN_FILENO);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
        }
        execl(self.constData(), self.constData(), "--serve-clipboard", nullptr);
        _exit(127);
    }

    close(fds[0]);
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

    Manager manager;
    int rc = 1;
    // The manager binds on the first event-loop pass.
    QTimer::singleShot(0, &app, [&] {
        auto *display = QtWaylandClient::QWaylandIntegration::instance()->display();
        auto *seat = display ? display->defaultInputDevice() : nullptr;
        if (!manager.isActive() || !seat) {
            QCoreApplication::exit(1);
            return;
        }
        auto *device = new QtWayland::zwlr_data_control_device_v1(
            manager.get_data_device(seat->wl_seat()));
        auto *source = new Source(manager.create_data_source(), png);
        source->offer(QStringLiteral("image/png"));
        device->set_selection(source->object());
        rc = 0;
    });
    // Returns when another client takes the selection (`cancelled`).
    int exitCode = app.exec();
    return exitCode ? exitCode : rc;
}
