// SPDX-License-Identifier: LGPL-3.0-or-later

#include "miscutills.h"

#include <QSocketNotifier>

#include <sys/signalfd.h>
#include <unistd.h>
#include "outputtypes.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <QPainter>
#include <QRegularExpression>
#include <QIcon>

namespace miscutills {
    void call_dbus(QString path, const QVariantList &args){
        QString slot = path.split("/").last();
        path = path.remove("/" + slot);

        if (QDBusConnection::sessionBus().isConnected()){
            QDBusInterface iface("org.forest", "/org/" + path, "", QDBusConnection::sessionBus());
            if (iface.isValid()) iface.callWithArgumentList(QDBus::Block, slot, args);
            else qWarning() << "DBus call failed:" << QDBusConnection::sessionBus().lastError().message();
        }
        else {
            qCritical() << "Cannot connect to the D-Bus session bus";
        }
    }

    void call_dbus_async(QString path, const QVariantList &args){
        QString slot = path.split("/").last();
        path = path.remove("/" + slot);

        // No QDBusInterface: its constructor introspects the target synchronously.
        QDBusMessage message = QDBusMessage::createMethodCall("org.forest", "/org/" + path, QString(), slot);
        message.setArguments(args);
        QDBusConnection::sessionBus().asyncCall(message);
    }

    QString run_shell_command(QString command){
        QProcess process;
        process.start("bash", QStringList() << "-c" << command);
        process.waitForFinished();
        return process.readAllStandardOutput();
    }

    QSize get_iconsize_stylesheet(QString selector, QString stylesheet){
            int size = 16;
            QRegularExpression re(selector+" {(\n[^}]*)*");
            QRegularExpressionMatch match = re.match(stylesheet);
            if (match.hasMatch()) {
                QString matched = match.captured(0);
                QStringList items = matched.split(";");
                foreach(QString item, items){
                    QStringList key_value = item.split(":");
                    if(key_value[0].contains("icon-size")){
                        QString s = key_value[1].remove("px").trimmed();
                        size = s.toInt();
                    }
                }
            }
            return QSize(size, size);
        }

    QString pad_with_zeros(int number){
        if (number < 10) return "000" + QString::number(number);
        else if (number < 100) return "00" + QString::number(number);
        else if (number < 1000) return "0" + QString::number(number);
        else return QString::number(number);
    }

    QIcon make_color_icon(QColor color){
        QImage i(30,20, QImage::Format_ARGB32);
        i.fill(color);
        return QIcon(QPixmap::fromImage(i));
    }

    QString color_to_string(QColor color){
        return QStringList({
            QString::number(color.red()),
            QString::number(color.green()),
            QString::number(color.blue()),
        }).join(",");
    }

    QColor string_to_color(QString color_string){
        QStringList channels_list = color_string.split(",");
        return QColor(channels_list.at(0).toInt(), channels_list.at(1).toInt(), channels_list.at(2).toInt());
    }
}

RunOnce::RunOnce(int delay, QObject *parent) : QObject(parent){
    timer.setSingleShot(true);
    timer.setInterval(delay);
    connect(&timer, &QTimer::timeout, this, &RunOnce::activated);
}

void RunOnce::try_activate(){
    timer.start();
}

ScreenTracker::ScreenTracker(QObject *parent) : QObject(parent){
    foreach (QScreen *screen, qApp->screens()){
        tracked_screens << screen;
        watch(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded, this, [this](QScreen *screen){
        watch(screen);
        runner.try_activate();
    });
    connect(qApp, &QGuiApplication::screenRemoved, &runner, &RunOnce::try_activate);
    connect(&runner, &RunOnce::activated, this, &ScreenTracker::handle_change);

    last_primary = primary_name();
    QDBusConnection::sessionBus().connect("org.forest", "/org/forest/displays", "org.forest.displays",
                                          "primaryChanged", this, SLOT(handle_primary_setting()));
}

QString ScreenTracker::primary_name() const{
    QScreen *screen = primary();
    return screen ? screen->name() : QString();
}

// Not debounced: a primary-only change has no screen change behind it.
void ScreenTracker::handle_primary_setting(){
    if (runner.is_pending()) return; // handle_change will pick it up
    const QString name = primary_name();
    if (name == last_primary) return;
    last_primary = name;
    emit primary_changed();
}

QScreen* ScreenTracker::primary(){
    const QList<QScreen*> screens = qApp->screens();
    QStringList names;
    QList<QPoint> positions;
    foreach (QScreen *screen, screens){
        names << screen->name();
        positions << screen->geometry().topLeft();
    }
    const int index = outputs::pickPrimary(outputs::savedPrimary(), names, positions);
    return index < 0 ? nullptr : screens[index];
}

void ScreenTracker::watch(QScreen *screen){
    connect(screen, &QScreen::geometryChanged, &runner, &RunOnce::try_activate);
}

void ScreenTracker::handle_change(){
    last_primary = primary_name();
    QList<QScreen*> screens = qApp->screens();
    bool replaced = screens.length() != tracked_screens.length();
    foreach (const QPointer<QScreen> &screen, tracked_screens){
        if (!screen || !screens.contains(screen.data()))
            replaced = true;
    }

    if (replaced){
        tracked_screens.clear();
        foreach (QScreen *screen, screens)
            tracked_screens << screen;
        emit screens_replaced();
    }
    else {
        emit geometry_changed();
    }
}

sigset_t block_signals(std::initializer_list<int> signums)
{
    sigset_t mask;
    sigemptyset(&mask);
    for (int signum : signums)
        sigaddset(&mask, signum);
    pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    return mask;
}

QObject *watch_signals(const sigset_t &mask, QObject *parent, std::function<void()> handler)
{
    const int fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (fd < 0)
        return nullptr;
    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, parent);
    QObject::connect(notifier, &QSocketNotifier::activated, notifier, [fd, handler = std::move(handler)] {
        signalfd_siginfo info;
        while (read(fd, &info, sizeof info) == sizeof info) {}
        handler();
    });
    QObject::connect(notifier, &QObject::destroyed, notifier, [fd] { close(fd); });
    return notifier;
}
