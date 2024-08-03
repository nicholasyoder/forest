#include "miscutills.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcess>
#include <QPainter>
#include <QRegularExpression>

miscutills::miscutills()
{
}

void miscutills::call_dbus(QString path){
    QString slot = path.split("/").last();
    path = path.remove("/" + slot);

    if (QDBusConnection::sessionBus().isConnected()){
        QDBusInterface iface("org.forest", "/org/" + path, "", QDBusConnection::sessionBus());
        if (iface.isValid()) iface.call(slot);
        else fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
    else {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\nTo start it, run:\n\teval `dbus-launch --auto-syntax`\n");
    }
}

QString miscutills::run_shell_command(QString command){
    QProcess process;
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();
    return process.readAllStandardOutput();
}

QSize miscutills::get_iconsize_stylesheet(QString selector, QString stylesheet){
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

QImage* miscutills::get_wallpaper_scaled(QImage *source_image, WALLPAPER_MODE mode, QSize target_size){
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

QImage* miscutills::get_wallpaper_scaled(QString wallpaper_file, WALLPAPER_MODE mode, QSize target_size){
    QImage *wallpaper_image = new QImage(wallpaper_file);
    return get_wallpaper_scaled(wallpaper_image, mode, target_size);
}
