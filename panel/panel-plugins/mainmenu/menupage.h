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
