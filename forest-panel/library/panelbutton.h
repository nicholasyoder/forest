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

#ifndef PANELBUTTON_H
#define PANELBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QSettings>
#include <QDebug>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QStyleOptionButton>

#include <qt5xdg/XdgIcon>

class panelbutton : public QFrame
{
    Q_OBJECT

public:
    panelbutton(){ setObjectName("panelButton"); setProperty("buttontype", "Custom");}

    void setupIconButton(QIcon icon, int customiconsize = 0){
        setIcon(icon);
        setupiconsize(customiconsize);
        setProperty("buttontype", "Icon");
    }

    void setupIconButton(QString icon, int customiconsize = 0){
        setIcon(geticon(icon));
        setupiconsize(customiconsize);
        setProperty("buttontype", "Icon");
    }

    void setupTextButton(QString text){
        setText(text);
        setProperty("buttontype", "Text");
    }

    void setupIconAndTextButton(QString text, QString icon, int customiconsize = 0){
        setText(text);
        setIcon(geticon(icon));
        setupiconsize(customiconsize);
        setProperty("buttontype", "IconAndText");
    }

    void setupIconAndTextButton(QString text, QIcon icon, int customiconsize = 0){
        setText(text);
        setIcon(icon);
        setupiconsize(customiconsize);
        setProperty("buttontype", "IconAndText");
    }


    QIcon geticon(QString icon){
        QIcon ico;
        if (QIcon::hasThemeIcon(icon)){ico = QIcon::fromTheme(icon);}
        else{
            if (icon.startsWith("/")){ico = QIcon(icon);}
            else{
                QFile icofile("/usr/share/pixmaps/" + icon + ".png");
                if (icofile.exists()){ico = QIcon("/usr/share/pixmaps/" + icon + ".png");}
                else{
                    QFile icofile2("/usr/share/pixmaps/" + icon);
                    if (icofile2.exists()){ico = QIcon("/usr/share/pixmaps/" + icon);}
                    else{
                        QFile icofile3("/usr/share/pixmaps/" + icon + ".svg");
                        if (icofile3.exists()){ico = QIcon("/usr/share/pixmaps/" + icon + ".svg");}
                        else{ico = XdgIcon::fromTheme(icon, QIcon::fromTheme("unknown"));}
                    }
                }
            }
        }
        return ico;
    }

    void setupiconsize(int customiconsize){
        int iconsize;
        if (customiconsize != 0){ iconsize = customiconsize;}
        else{ iconsize = 22;}
        setIconSize(QSize(iconsize, iconsize));
    }

    QString text(){return bttext;}
    QIcon icon(){return bticon;}
    QSize iconSize(){return bticonsize;}
    bool isDown(){return btdown;}
    bool isMouseOver(){return btmouseover;}

    void setText(QString text){
        bttext = text;
        if (property("buttontype") == "Text") {
            QStyleOptionButton option;
            option.initFrom(this);
            option.text = text;
            setStyleSheet("min-width: " + QString::number(option.fontMetrics.horizontalAdvance(text)) + "px;");
        }
        else {
            update();
        }
    }
    void setIcon(QIcon icon){bticon = icon; update();}
    void setIcon(QString icon){bticon = geticon(icon); update();}
    void setIconSize(QSize size){bticonsize = size; update();}
    void setDown(bool down){btdown = down; update();}
    void setMouseOver(bool mouseover){btmouseover = mouseover; update();}

signals:
    void leftclicked();
    void rightclicked();
    void mouseReleased(QMouseEvent *event);
    void enterevent();
    void leaveevent();

protected:
    void enterEvent(QEvent *){setMouseOver(true); emit enterevent();}
    void leaveEvent(QEvent *){setMouseOver(false); emit leaveevent();}
    void mousePressEvent(QMouseEvent *){setDown(true);}
    void mouseReleaseEvent(QMouseEvent *event){
        setDown(false);
        if (event->button() == Qt::LeftButton)
            emit leftclicked();
        else if (event->button() == Qt::RightButton)
            emit rightclicked();

        emit mouseReleased(event);
    }

    void paintEvent(QPaintEvent *)
    {
        QStyleOptionButton option;
        option.initFrom(this);
        if (isDown())
            option.state |= QStyle::State_Sunken;
        else if (isMouseOver())
            option.state |= QStyle::State_MouseOver;
        else
            option.state |= QStyle::State_Raised;

        if (text()!="" && !icon().isNull())
            option.text = option.fontMetrics.elidedText(text(), Qt::ElideRight, style()->subElementRect(QStyle::SE_PushButtonContents, &option, this).width() - iconSize().width()-2);
        else
            option.text = text();

        option.icon = icon();
        option.iconSize = iconSize();

        QPainter painter(this);
        style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
    }

private:
    QString bttext;
    QIcon bticon;
    QSize bticonsize;
    bool btdown = false;
    bool btmouseover = false;
};

#endif // PANELBUTTON_H
