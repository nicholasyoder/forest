#ifndef WALLPAPERWIDGET_H
#define WALLPAPERWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>

enum ImageMode { Fill, Fit, Stretch, Tile, Center};

class wallpaperwidget : public QWidget
{
    Q_OBJECT
public:
    wallpaperwidget(QImage *image, ImageMode imode = Fill);

public slots:
    void setwallpaper(QImage *image){wallpaper = image; setupwallpaper();}
    void setimagemode(ImageMode imode){imagemode = imode; setupwallpaper();}
    void setcontextmenu(QMenu *contextmenu){cmenu = contextmenu;}

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *){setupwallpaper();}

private slots:
    void paintFill();
    void paintFit();
    void paintStretch();
    void paintTile();
    void paintCenter();

    void setupwallpaper();

private:
    QImage *wallpaper = new QImage();
    QImage *scaledwallpaper = new QImage();
    QMenu *cmenu = nullptr;
    ImageMode imagemode;
};

#endif // WALLPAPERWIDGET_H
