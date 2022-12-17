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

#include "menupage.h"

#include <QScrollBar>
#include <QDebug>

menupage::menupage(QList<menuitem*> items)
{
    foreach(menuitem *item, items){
        itemVLayout->addWidget(item);
        connect(item, &menuitem::focused, this, &menupage::itemFocused);
    }

    itemVLayout->setMargin(0);
    itemVLayout->setSpacing(0);
    itemVLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scrollWidget->setLayout(itemVLayout);
    scrollWidget->setObjectName("panelMainMenuScrollWidget");
    scrollArea->setObjectName("panelMainMenuListWidget");

    setLayout(baseVLayout);
    baseVLayout->addWidget(scrollArea);
    baseVLayout->setMargin(0);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFocusPolicy(Qt::NoFocus);
}

menupage::~menupage()
{
    QLayoutItem *child;
    while((child = itemVLayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }

    delete itemVLayout;
    delete scrollWidget;
    delete scrollArea;
    delete baseVLayout;
}

void menupage::itemFocused(menuitem *item){
    scrollArea->ensureWidgetVisible(item, 0, 1);
}
