#ifndef FUTILS_H
#define FUTILS_H

#include <QString>
#include <QSize>
#include <QRegularExpressionMatch>
#include <QDebug>
#include <QtGui/QImage>
#include <QtGui/QPainter>

enum WALLPAPER_MODE { Fill, Fit, Stretch, Tile, Center};

class futils
{
public:
    futils(){}

    static QSize get_iconsize_stylesheet(QString selector, QString stylesheet){
        int size = 16;
        QRegularExpression re(selector+" {(\n[^}]*)*");
        QRegularExpressionMatch match = re.match(stylesheet);
        if (match.hasMatch()) {
            QString matched = match.captured(0);
            //qDebug() << matched;
            QStringList items = matched.split(";");
            foreach(QString item, items){
                //qDebug() << item;
                QStringList key_value = item.split(":");
                if(key_value[0].contains("icon-size")){
                    QString s = key_value[1].remove("px").trimmed();
                    size = s.toInt();
                }
            }
        }
        return QSize(size, size);
    }

    static QImage* get_wallpaper_scaled(QImage *source_image, WALLPAPER_MODE mode, QSize target_size){
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

    static QImage* get_wallpaper_scaled(QString wallpaper_file, WALLPAPER_MODE mode, QSize target_size){
        QImage *wallpaper_image = new QImage(wallpaper_file);
        return get_wallpaper_scaled(wallpaper_image, mode, target_size);
    }
};

#endif // FUTILS_H
