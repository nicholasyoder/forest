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

#ifndef CATLISTWIDGET_H
#define CATLISTWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QUuid>

class catlistitem : public QFrame
{
    Q_OBJECT
public:
    catlistitem(QUuid id, QString text, QIcon icon);

public slots:
    void updatepressed(QUuid id);
    void activate(){ pressed = true; update(); emit clicked(item_id); }
    QString text(){ return item_text; }

signals:
    void clicked(QUuid id);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);

private:
    bool highlight = false;
    bool pressed = false;
    bool mousepressed = false;
    QString item_text;
    QIcon item_icon;
    QUuid item_id;
};


class catlistwidget : public QFrame
{
    Q_OBJECT
public:
    catlistwidget();

signals:
    //void currentRowChanged(int row);
    void currentRowChanged(QUuid id);

public slots:
    void clear();
    void additem(QUuid id, QString text, QIcon icon);
    void addseperator(QString text);
    void setcurrentitem(QString itemtext){handleitemclicked(itemtext);}
    QList<catlistitem*> items(){ return item_list; }

private slots:
    void handleitemclicked(QUuid id);

private:
    QVBoxLayout *basevlayout = new QVBoxLayout(this);
    QList<catlistitem*> item_list;
    QList<QLabel*> seperator_list;
};

#endif // CATLISTWIDGET_H
