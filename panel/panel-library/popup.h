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

#ifndef POPUPBOX_H
#define POPUPBOX_H

#include <QWidget>
#include <QSettings>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QVBoxLayout>

enum PositionpPolicy { CenteredOnWidget, EdgeAlignedOnWidget, CenteredOnMouse, EdgeAlignedOnMouse };

class popup : public QWidget
{
    Q_OBJECT
public:
    popup(QLayout *layout, QWidget *launcherw, PositionpPolicy policy){
        contentlayout = layout;
        launcherwidget = launcherw;
        pospolicy = policy;

        setWindowFlags(Qt::Popup);
        setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout *hlayout = new QHBoxLayout(this);
        hlayout->setMargin(0);
        popupQFrame = new QFrame;
        popupQFrame->setObjectName("popup");
        popupQFrame->setLayout(contentlayout);
        hlayout->addWidget(popupQFrame);
    }

    QFrame *popupQFrame = nullptr;

signals:
    void keypressed(QKeyEvent *event);
    void mousereleased(QMouseEvent *event);

public slots:
    void showpopup(){
        positionOnLauncher();
        show();
    }

    void closepopup(){this->close();}

    void focuschild(bool next = true)
    {
        if (next)
            this->focusNextChild();
        else
            this->focusPreviousChild();
    }

    void changelauncher(QWidget *launcher){launcherwidget = launcher;}

    void positionOnLauncher(){
        QRect launcherRect(launcherwidget->mapToGlobal(QPoint(0,0)), launcherwidget->size());
        QRect popupRect;
        if (this->maximumSize() != QSize(16777215, 16777215)) popupRect.setSize(this->size());
        else popupRect.setSize(this->sizeHint());

        int edgeoffset = getEdgeOffset(popupRect, launcherRect, QCursor::pos().x());

        QRect panelRect = getpanelRect();
        QString panelpos = psettings->value("position").toString().toLower();
        if (panelpos == "top")
            popupRect.moveTo(QPoint(launcherRect.x() + edgeoffset, panelRect.height()));
        else //if (panelpos == "bottom")
            popupRect.moveTo(QPoint(launcherRect.x() + edgeoffset, panelRect.y() - popupRect.height()));
        /*else if (panelpos == "right")
            popupRect.moveTo(QPoint(panelRect.width() - popupRect.height(), launcherRect.x() + edgeoffset));
        else if (panelpos == "left")
            popupRect.moveTo(QPoint(panelRect.width(), launcherRect.x() + edgeoffset));*/

        QSize screensize(qApp->desktop()->screenGeometry(launcherwidget).size());
        QPoint newpos(popupRect.topLeft());

        if (popupRect.x() + popupRect.width() > screensize.width())
            newpos.setX(screensize.width() - popupRect.width());
        if (popupRect.y() + popupRect.height() > screensize.height())
            newpos.setY(screensize.height() - popupRect.height());
        if (popupRect.x() < 0)
            newpos.setX(0);
        if (popupRect.y() < 0)
            newpos.setY(0);

        move(newpos);
    }

protected:
    void keyPressEvent(QKeyEvent *event){emit keypressed(event);}//so the object controlling the popup can use keystokes
    void mouseReleaseEvent(QMouseEvent *event){emit mousereleased(event);}//and mouse clicks

private slots:
    QRect getpanelRect(){
        QWidget *panel = getpanelwidget();
        if (panel) return QRect(panel->mapToGlobal(panel->geometry().topLeft()), panel->size());
        else return QRect(100,100,100,100);
    }

    int getEdgeOffset(QRect popupRect, QRect launcherRect, int mousepos){
        if (pospolicy == CenteredOnWidget)
            return -(popupRect.width() / 2 - launcherRect.width() / 2);
        else if (pospolicy == EdgeAlignedOnWidget)
            return 0;
        else if (pospolicy == CenteredOnMouse)
            return mousepos - launcherRect.x() - popupRect.width() / 2;
        else// if (pospolicy == EdgeAlignedOnMouse)
            return mousepos - launcherRect.x();
    }

    QWidget* getpanelwidget() {
        QWidget *parent = launcherwidget->parentWidget();
        while (1) {
            if (parent){
                if (parent->objectName() == "panel") return parent;
                else parent = parent->parentWidget();
            }
            else return nullptr;
        }
    }

private:
    //bool allowclose = false;
    QLayout *contentlayout;
    QWidget *launcherwidget;
    PositionpPolicy pospolicy;
    QSettings *psettings = new QSettings("Forest","Panel");
};

#endif // POPUPBOX_H
