/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef POPUPMENU_H
#define POPUPMENU_H

#include <QObject>
#include <QVBoxLayout>
#include <QPushButton>

#include "popup.h"
#include "panelbutton.h"

//menuitem class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class pmenuitem : public QPushButton
{
    Q_OBJECT

public:
    pmenuitem(QString text, QIcon icon = QIcon()){
        setText(text);
        if (!icon.isNull())
            setIcon(icon);

        setObjectName("popupMenuItem");
    }
};

//menuseperator class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class menuseperator : public QFrame
{
    Q_OBJECT

public:
    menuseperator(){ setObjectName("popupMenuSeperator"); }
};

//popupmenu class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class popupmenu : public QObject
{
    Q_OBJECT

public:
    popupmenu(QWidget *launcherw, PositionpPolicy policy)
    {
        vlayout = new QVBoxLayout;
        vlayout->setMargin(0);
        vlayout->setSpacing(0);
        popupw = new popup(vlayout, launcherw, policy);
        connect(popupw, &popup::keypressed, this, &popupmenu::handlekeypress);
    }

    popup *popupw;

public slots:
    void show(){popupw->showpopup();}
    void close(){popupw->closepopup();}

    void additem(pmenuitem *item){
        vlayout->addWidget(item);
        connect(item, &QPushButton::clicked, this, &popupmenu::close);
    }

    void addseperator(){
        menuseperator *sep = new menuseperator;
        vlayout->addWidget(sep);
    }

    void handlekeypress(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_Escape)
            this->close();
        else if (event->key() == Qt::Key_Up)
            popupw->focuschild(false);
        else if (event->key() == Qt::Key_Down)
            popupw->focuschild(true);
    }

    void changelauncher(QWidget *launcher) {popupw->changelauncher(launcher);}

private:

    QVBoxLayout *vlayout;
};

#endif // POPUPMENU_H
