// SPDX-License-Identifier: LGPL-3.0-or-later

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
        vlayout->setContentsMargins(QMargins(0,0,0,0));
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
