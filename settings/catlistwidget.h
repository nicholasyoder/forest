// SPDX-License-Identifier: LGPL-3.0-or-later

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
    catlistitem(QUuid id, QString text, QIcon icon, bool has_subitems = false);

public slots:
    void updatepressed(QUuid id);
    void activate(){ pressed = true; update(); emit clicked(item_id); }
    QString text(){ return item_text; }

signals:
    void clicked(QUuid id);

protected:
    void enterEvent(QEnterEvent *);
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
    bool item_has_subitems;
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
    void additem(QUuid id, QString text, QIcon icon, bool has_subitems = false);
    void addseperator(QString text);
    void setcurrentitem(QString itemtext){handleitemclicked(QUuid::fromString(itemtext));}
    QList<catlistitem*> items(){ return item_list; }

private slots:
    void handleitemclicked(QUuid id);

private:
    QVBoxLayout *basevlayout = new QVBoxLayout(this);
    QList<catlistitem*> item_list;
    QList<QLabel*> seperator_list;
};

#endif // CATLISTWIDGET_H
