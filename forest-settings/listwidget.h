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

#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

class listwidget : public QFrame
{
    Q_OBJECT
public:
    listwidget();

signals:
    void currentRowChanged(int row);
    void updatepressed(QString currentitemtext);

public slots:
    void additem(QString text, QIcon icon);
    void addseperator(QString text);
    void setcurrentitem(QString itemtext){handleitemclicked(itemtext);}

private slots:
    void handleitemclicked(QString itemtext);

private:
    QVBoxLayout *basevlayout = new QVBoxLayout(this);
    QStringList itemlist;
};

class listitem : public QWidget
{
    Q_OBJECT
public:
    listitem(QString text, QIcon icon);

public slots:
    void updatepressed(QString currentitemtext);

signals:
    void clicked(QString text);

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
    QString itemtext;
};

#endif // LISTWIDGET_H
