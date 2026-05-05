// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MENUPAGE_H
#define MENUPAGE_H

#include <QWidget>
#include <QHash>
#include <QScrollArea>
#include <QVBoxLayout>

#include "menuitem.h"

class menupage : public QWidget
{
    Q_OBJECT

public:
    menupage(QList<menuitem*> items);
    ~menupage();

private slots:
    void itemFocused(menuitem *item);

private:
    QVBoxLayout *baseVLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    QFrame *scrollWidget = new QFrame;
    QVBoxLayout *itemVLayout = new QVBoxLayout;

};
#endif // MENUPAGE_H
