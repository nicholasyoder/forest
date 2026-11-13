// SPDX-License-Identifier: LGPL-3.0-or-later

#include "capturecore.h"
#include "clipboard.h"
#include "flogger.h"

#include "extforeigntoplevelhandle.h"
#include "foreigntoplevelhandle.h"
#include "screengrab.h"
#include "toplevelcapture.h"
#include "topleveltracker.h"

#include <QElapsedTimer>
#include <QGuiApplication>
#include <QScreen>
#include <QTextStream>
#include <QTimer>

namespace {

enum class Mode { None, Full, Monitor, Window, Region };

struct Args {
    Mode mode = Mode::None;
    QString monitorName;
    int delay = 0;
    capturecore::Options opts;
};

int usage(const QString &problem = {}) {
    QTextStream err(stderr);
    if (!problem.isEmpty())
        err << "forest-screenshot: " << problem << "\n";
    err << "usage: forest-screenshot [--full | --monitor [NAME] | --window | --region]\n"
           "                         [--delay N] [--no-save] [--no-clipboard] [--output FILE]\n";
    return 2;
}

// Returns an exit code, or -1 on success.
int parse(const QStringList &a, Args *out) {
    for (int i = 1; i < a.size(); ++i) {
        const QString &s = a[i];
        auto setMode = [&](Mode m) {
            if (out->mode != Mode::None)
                return false;
            out->mode = m;
            return true;
        };
        if (s == QLatin1String("--full")) {
            if (!setMode(Mode::Full)) return usage("only one capture mode allowed");
        } else if (s == QLatin1String("--window")) {
            if (!setMode(Mode::Window)) return usage("only one capture mode allowed");
        } else if (s == QLatin1String("--region")) {
            if (!setMode(Mode::Region)) return usage("only one capture mode allowed");
        } else if (s == QLatin1String("--monitor")) {
            if (!setMode(Mode::Monitor)) return usage("only one capture mode allowed");
            if (i + 1 < a.size() && !a[i + 1].startsWith(QLatin1String("--")))
                out->monitorName = a[++i];
        } else if (s == QLatin1String("--delay")) {
            bool ok = false;
            if (i + 1 >= a.size() || (out->delay = a[++i].toInt(&ok), !ok || out->delay < 0))
                return usage("--delay needs a non-negative number of seconds");
        } else if (s == QLatin1String("--output")) {
            if (i + 1 >= a.size()) return usage("--output needs a file");
            out->opts.outputFile = a[++i];
        } else if (s == QLatin1String("--no-save")) {
            out->opts.save = false;
        } else if (s == QLatin1String("--no-clipboard")) {
            out->opts.clipboard = false;
        } else {
            return usage("unknown argument " + s);
        }
    }
    if (!out->opts.save && !out->opts.clipboard)
        return usage("--no-save and --no-clipboard together leave nothing to do");
    return -1;
}

// Grabs every output in parallel; calls done with the composited image (null on failure).
void grabScreens(const QList<QScreen *> &screens, QObject *ctx, std::function<void(const QImage &)> done) {
    struct State {
        QList<capturecore::Shot> shots;
        int pending;
        bool failed = false;
    };
    auto *st = new State{{}, int(screens.size())};
    for (QScreen *screen : screens) {
        ScreenGrab *grab = ScreenGrab::grabOutput(screen, ctx);
        QObject::connect(grab, &ScreenGrab::finished, ctx, [=](const QImage &img) {
            if (img.isNull())
                st->failed = true;
            else
                st->shots.append({screen, img});
            if (--st->pending > 0)
                return;
            QImage result = st->failed ? QImage() : capturecore::compose(st->shots);
            delete st;
            done(result);
        });
    }
}

void finish(const QImage &image, const capturecore::Options &opts) {
    // The clipboard helper is detached, so exiting right away is safe.
    QCoreApplication::exit(capturecore::deliver(image, opts));
}

void capture(const Args &args, ToplevelTracker *tracker) {
    QTextStream err(stderr);
    auto failWith = [&](const QString &why) {
        err << "forest-screenshot: " << why << Qt::endl;
        capturecore::notify(QStringLiteral("Screenshot failed"), why, true);
        QCoreApplication::exit(1);
    };

    switch (args.mode) {
    case Mode::Full:
        grabScreens(QGuiApplication::screens(), qApp, [=](const QImage &img) { finish(img, args.opts); });
        break;
    case Mode::Monitor: {
        QScreen *target = nullptr;
        if (args.monitorName.isEmpty()) {
            target = QGuiApplication::primaryScreen(); // picker comes with the overlay
        } else {
            for (QScreen *s : QGuiApplication::screens())
                if (s->name() == args.monitorName)
                    target = s;
            if (!target)
                return failWith("no output named " + args.monitorName);
        }
        grabScreens({target}, qApp, [=](const QImage &img) { finish(img, args.opts); });
        break;
    }
    case Mode::Window: {
        // The wlr<->ext handle pairing completes asynchronously; poll briefly.
        auto *timer = new QTimer(qApp);
        auto *clock = new QElapsedTimer;
        clock->start();
        QObject::connect(timer, &QTimer::timeout, qApp, [=] {
            ExtForeignToplevelHandle *ext = nullptr;
            for (ForeignToplevelHandle *h : tracker->toplevels())
                if (h->isActivated() && h->extHandle())
                    ext = h->extHandle();
            if (!ext && clock->elapsed() < 2000)
                return;
            timer->stop();
            if (!ext) {
                QTextStream(stderr) << "forest-screenshot: no active window\n";
                capturecore::notify(QStringLiteral("Screenshot failed"), QStringLiteral("No active window"), true);
                QCoreApplication::exit(1);
                return;
            }
            ScreenGrab *grab = ScreenGrab::grabWindow(ext, qApp);
            QObject::connect(grab, &ScreenGrab::finished, qApp,
                             [=](const QImage &img) { finish(img, args.opts); });
        });
        timer->start(50);
        break;
    }
    default:
        err << "forest-screenshot: this mode is not implemented yet\n";
        QCoreApplication::exit(2);
    }
}

} // namespace

int main(int argc, char *argv[]) {
    // Clipboard helper: re-exec of ourselves from clipboard::copyPng().
    if (argc == 2 && QByteArrayView(argv[1]) == "--serve-clipboard")
        return clipboard::serve();

    QGuiApplication app(argc, argv);
    FLogger::install("screenshot");

    Args args;
    if (int rc = parse(app.arguments(), &args); rc >= 0)
        return rc;
    if (args.mode == Mode::None)
        return usage("the capture UI is not implemented yet; pass a mode");

    ToplevelCapture::preload();
    ToplevelTracker *tracker = args.mode == Mode::Window ? new ToplevelTracker(&app) : nullptr;
    // One pass lets the capture globals bind before the first isSupported().
    QTimer::singleShot(args.delay * 1000 + 100, &app, [&] { capture(args, tracker); });
    return app.exec();
}
