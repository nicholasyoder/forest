// SPDX-License-Identifier: LGPL-3.0-or-later

#include "miscutills.h"

#include <QDBusConnection>
#include <QDBusInterface>
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

    QImage* get_wallpaper_scaled(QImage *source_image, WALLPAPER_MODE mode, QSize target_size){
        // Create an image the exact size of the target with the
        // wallpaper image painted on it in the manner specified by `mode`

        QImage wallpaper_image = *source_image;
        int target_width = target_size.width();
        int target_height = target_size.height();
        QImage *scaled_wallpaper = new QImage(target_width, target_height, wallpaper_image.format());
        scaled_wallpaper->fill(Qt::black);
        QRectF target, source;

        switch (mode){
            case Fill:{
                wallpaper_image = wallpaper_image.scaled(target_width, target_height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                int x = 0, y = 0;
                if (wallpaper_image.width() > target_width)
                    x = (wallpaper_image.width() - target_width) / 2;
                else if (wallpaper_image.height() > target_height)
                    y = (wallpaper_image.height() - target_height) / 2;
                target = QRectF(0, 0, target_width, target_height);
                source = QRectF(x, y, target_width, target_height);
                break;
            }
            case Fit:{
                wallpaper_image = wallpaper_image.scaled(target_width, target_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                int x = target_width/2 - wallpaper_image.width()/2;
                int y = target_height/2 - wallpaper_image.height()/2;
                target = QRectF(x, y, wallpaper_image.width(), wallpaper_image.height());
                source = QRectF(0, 0, wallpaper_image.width(), wallpaper_image.height());
                break;
            }
            case Stretch:{
                wallpaper_image = wallpaper_image.scaled(target_width, target_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                target = QRectF(0.0, 0.0, target_width, target_height);
                source = QRectF(0.0, 0.0, wallpaper_image.width(), wallpaper_image.height());
                break;
            }
            case Tile:{

                break;
            }
            case Center:{
                int x = target_width/2 - wallpaper_image.width()/2;
                int y = target_height/2 - wallpaper_image.height()/2;
                target = QRectF(x, y, wallpaper_image.width(), wallpaper_image.height());
                source = QRectF(0, 0, wallpaper_image.width(), wallpaper_image.height());
                break;
            }
        }

        QPainter painter(scaled_wallpaper);
        painter.drawImage(target, wallpaper_image, source);

        return scaled_wallpaper;
    }

    QImage* get_wallpaper_scaled(QString wallpaper_file, WALLPAPER_MODE mode, QSize target_size){
        QImage *wallpaper_image = new QImage(wallpaper_file);
        return get_wallpaper_scaled(wallpaper_image, mode, target_size);
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

RunOnce::RunOnce(int delay){
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
    QList<QScreen*> screens = qApp->screens();
    QString name = QSettings("Forest", "Forest").value("display/primary_screen").toString();
    QScreen *best = nullptr;
    foreach (QScreen *screen, screens){
        if (!name.isEmpty() && screen->name() == name)
            return screen;
        QPoint pos = screen->geometry().topLeft();
        if (!best || pos.x() < best->geometry().x() || (pos.x() == best->geometry().x() && pos.y() < best->geometry().y()))
            best = screen;
    }
    return best;
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
