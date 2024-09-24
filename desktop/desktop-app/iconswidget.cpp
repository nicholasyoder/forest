#include "iconswidget.h"

iconswidget::iconswidget(QSize size, QRect usable)
{
    screensize = size;
    usablespace = usable;

    dbox->setParent(this);
    dbox->hide();

    setAcceptDrops(true);
}

void iconswidget::addicon(desktopicon *icon, int column, int row)
{
    if (!hasiconat(column, row))
        moveicon(icon, QPoint(column,row));
    else
        moveicon(icon, firstemptycell());

    connect(icon, &desktopicon::sigselected, this, &iconswidget::handleiconselected);
    connect(icon, &desktopicon::sigdragged, this, &iconswidget::handleicondragged);
    connect(icon, &desktopicon::textchanged, this, &iconswidget::handleicontextchanged);

    icon->setParent(this);
    icon->show();
}

void iconswidget::removeicon(desktopicon *icon)
{
    iconXYposhash.remove(point2string(icon->pos()));
    delete icon;
}

void iconswidget::removeall()
{
    foreach (desktopicon *icon, iconXYposhash)
        removeicon(icon);
}

void iconswidget::selectall()
{
    inmultiselectmode = true;
    foreach (desktopicon *icon, iconXYposhash)
        icon->setselected(true);
}

void iconswidget::seticonintexteditmode(QString ID)
{
    foreach (desktopicon *icon, iconXYposhash)
    {
        if (icon->getID() == ID)
        {
            icon->enterTextEditMode();
            return;
        }
    }
}

QList<desktopicon*> iconswidget::selectedicons()
{
    QList<desktopicon*> selected;
    foreach (desktopicon *icon, iconXYposhash)
    {
        if (icon->isSelected())
            selected.append(icon);
    }
    return selected;
}

void iconswidget::mousePressEvent(QMouseEvent *event)
{
    inmultiselectmode = false;
    handleiconselected("none");

    this->setFocus();

    if (event->button() == Qt::LeftButton)
    {
        leftbtdown = true;
        dragstartpoint = event->pos();
    }
}

void iconswidget::mouseReleaseEvent(QMouseEvent *event)
{
    inmultiselectmode = false;

    if (event->button() == Qt::LeftButton)
    {
        leftbtdown = false;
        dbox->hide();
    }
    else if (event->button() == Qt::RightButton)
    {
        event->ignore();//allow parent widget to get event and show menu
    }
    else if (event->button() == Qt::MidButton)
    {
        //qApp->quit();
    }
}

void iconswidget::mouseMoveEvent(QMouseEvent *event)
{
    if (leftbtdown == true)
    {
        inmultiselectmode = true;

        resizedragbox(event->pos());
        selecticonsinbox();
    }
}

void iconswidget::keyPressEvent(QKeyEvent *event)
{
    emit keypressed(event);
}

void iconswidget::keyReleaseEvent(QKeyEvent *event)
{
    emit keyreleased(event);
}

//this makes your cursor change when drag entering to show this widget can take the drop
void iconswidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

//handle files being dropped
void iconswidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        emit filesdropped(event->mimeData()->urls());
        event->acceptProposedAction();
    }
}

void iconswidget::handleiconselected(QString iconID)
{
    if (!inmultiselectmode)
    {
        foreach (desktopicon *icon, iconXYposhash)
        {
            if (icon->getID() != iconID)
                icon->setselected(false);
        }
    }
}

void iconswidget::handleicondragged(desktopicon *icon)
{
    int c = x2column(icon->x() + icon->width()/2);
    int r = y2row(icon->y() + icon->height()/2);

    if (!hasiconat(c,r))
    {
        icon->move(column2x(c), row2y(r));
        iconXYposhash.remove(point2string(icon->getposB4drag()));
        iconXYposhash.insert(point2string(icon->pos()), icon);
        tryhandleiconposchange();
    }
    else
    {
        icon->canceldrag();
    }
}

void iconswidget::handleiconposchange()
{
    QHash<QString, QString> poshash;

    foreach (desktopicon *icon, iconXYposhash)
    {
        poshash[icon->getID()] = point2string(pos2grid(icon->pos()));
    }
    emit iconposchange(poshash);
}

void iconswidget::tryhandleiconposchange()
{
    if (poschangetimer->isActive())
    {
        poschangetimer->stop();
        poschangetimer->start(200);
    }
    else
    {
        delete poschangetimer;
        poschangetimer = new QTimer;
        connect(poschangetimer, &QTimer::timeout, this, &iconswidget::handleiconposchange);
        poschangetimer->setSingleShot(true);
        poschangetimer->start(200);
    }

}

QPoint iconswidget::firstemptycell()
{
    int r=0,c=0;
    int totalrows = height()/100;
    int totalcolumns = width()/100;

    while (c < totalcolumns){
        while (r < totalrows){
            if (!hasiconat(c,r)){
                return QPoint(c, r);
            }
            r++;
        }
        c++;
        r = 0;
    }

    return QPoint(0,0);
}

void iconswidget::moveicon(desktopicon *icon, QPoint newgridpos)
{
    QPoint oldXYpos = icon->pos();

    if (oldXYpos != QPoint(0,0))
        iconXYposhash.remove(point2string(oldXYpos));

    icon->move(grid2pos(newgridpos));
    iconXYposhash[point2string(grid2pos(newgridpos))] = icon;

    tryhandleiconposchange();
}

void iconswidget::resizedragbox(QPoint cursorpos)
{
    int xpos = cursorpos.x();// - diconsview->x();
    int ypos = cursorpos.y();// - diconsview->y();

    if (dbox->isHidden())
    {
        dbox->move(dragstartpoint);
        dbox->resize(1,1);
        dbox->raise();
        dbox->show();
    }

    if (xpos > dragstartpoint.x())
    {
        if (ypos > dragstartpoint.y())
        {
            dbox->move(dragstartpoint);
            dbox->resize(xpos - dragstartpoint.x(), ypos - dragstartpoint.y());
        }
        else
        {
            dbox->move(dragstartpoint.x(), ypos);
            dbox->resize(xpos - dragstartpoint.x(), dragstartpoint.y() - ypos);
        }

    }
    else if (xpos < dragstartpoint.x())
    {
        if (ypos > dragstartpoint.y())
        {
            dbox->move(xpos, dragstartpoint.y());
            dbox->resize(dragstartpoint.x() - xpos, ypos - dragstartpoint.y());
        }
        else
        {
            dbox->move(xpos, ypos);
            dbox->resize(dragstartpoint.x() - xpos, dragstartpoint.y() - ypos);
        }
    }
}

void iconswidget::selecticonsinbox()
{
    QRect selectedarea = dbox->geometry();

    foreach (desktopicon *icon, iconXYposhash)
    {
        if (selectedarea.intersects(icon->geometry()))
        {
            if (icon->isSelected() == false)
                icon->setselected(true);
        }
        else if (icon->isSelected() == true)
        {
            icon->setselected(false);
        }
    }
}
