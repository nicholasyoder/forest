#ifndef ICONSWIDGET_H
#define ICONSWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QDebug>
#include <QMimeData>

#include "desktopicon.h"

class dragbox : public QFrame
{
    Q_OBJECT

public:
    dragbox(){setObjectName("desktopDragBox");}
};

class iconswidget : public QWidget
{
    Q_OBJECT
public:
    iconswidget(QSize size, QRect usable);

signals:
    void iconposchange(QHash<QString, QString> poshash);
    void icontextchanged(QString ID, QString newtext);
    void refreshrequest();
    void filesdropped(QList<QUrl> urls);
    void keypressed(QKeyEvent *event);
    void keyreleased(QKeyEvent *event);

public slots:
    void addicon(desktopicon *icon, int column = 0, int row = 0);
    void removeicon(desktopicon *icon);
    void removeall();
    void selectall();
    void seticonintexteditmode(QString ID);
    void setscreensize(QSize size){screensize = size;}
    void setusabledesktopspace(QRect usable){usablespace = usable;}
    void setinmultiselectmode(bool inmode){inmultiselectmode = inmode;}
    QList<desktopicon*> selectedicons();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void handleiconselected(QString iconID);
    void handleicondragged(desktopicon *icon);
    void handleiconposchange();
    void handleicontextchanged(QString ID, QString newtext){emit icontextchanged(ID, newtext);}
    void tryhandleiconposchange();
    QPoint firstemptycell();
    void moveicon(desktopicon *icon, QPoint newgridpos);
    void resizedragbox(QPoint cursorpos);
    void selecticonsinbox();

    int row2y(int row){return usablespace.y() + row*gridsize;}
    int column2x(int column){return usablespace.x() + column*gridsize;}
    int y2row(int y){return (y - usablespace.y())/100;}
    int x2column(int x){return (x - usablespace.x())/100;}
    QPoint pos2grid(QPoint pos){return QPoint(x2column(pos.x()), y2row(pos.y()));}
    QPoint grid2pos(QPoint gpos){return QPoint(column2x(gpos.x()), row2y(gpos.y()));}
    QString point2string(QPoint point){return QString::number(point.x())+","+QString::number(point.y());}
    QPoint string2point(QString string){QStringList list = string.split(","); return QPoint(list.first().toInt(), list.last().toInt());}
    bool hasiconat(int column, int row){return iconXYposhash.value(point2string(QPoint(column2x(column), row2y(row))));}

private:
    int spacing = 8;
    int gridsize = 100;
    QSize screensize;
    QRect usablespace;
    QHash<QString, desktopicon*> iconXYposhash;
    QTimer *poschangetimer = new QTimer;

    bool leftbtdown = false;
    QPoint dragstartpoint;
    dragbox *dbox = new dragbox;

    bool inmultiselectmode = false;
};
#endif // ICONSWIDGET_H
