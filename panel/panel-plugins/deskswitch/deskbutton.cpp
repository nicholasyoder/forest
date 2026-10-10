// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deskbutton.h"

#include <QStyle>

deskbutton::deskbutton(int desknumber)
{
    desknum = desknumber;
    setObjectName("deskButton");
    setProperty("active", false);

    for (int i = 0; i < 3; i++){
        QFrame *window = new QFrame(this);
        window->setObjectName("deskWindow");
        window->setAttribute(Qt::WA_TransparentForMouseEvents);
        window->hide();
        windows << window;
    }
}

void deskbutton::setactive(int num)
{
    const bool active = num == desknum;
    if (property("active").toBool() == active)
        return;

    setProperty("active", active);
    // QSS matches dynamic properties only at polish time; children use them via descendant selectors.
    style()->unpolish(this);
    style()->polish(this);
    for (QFrame *window : std::as_const(windows)){
        style()->unpolish(window);
        style()->polish(window);
    }
}

void deskbutton::setNumDeskWindows(int num)
{
    numDeskWindows = num;
    layoutWindows();
}

void deskbutton::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    layoutWindows();
}

void deskbutton::layoutWindows()
{
    static const QList<int> offsets[] = {{}, {0}, {-1, 1}, {-2, 0, 2}};
    const QList<int> &shown = offsets[qBound(0, numDeskWindows, 3)];

    const QRect area = contentsRect();
    const int size = area.width() / 2;
    QRect box(0, 0, size, size);
    box.moveCenter(area.center());

    for (int i = 0; i < windows.length(); i++){
        if (i < shown.length()){
            windows[i]->setGeometry(box.translated(shown[i], shown[i]));
            windows[i]->show();
        }
        else
            windows[i]->hide();
    }
}

void deskbutton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(desknum);

    event->ignore();
}
