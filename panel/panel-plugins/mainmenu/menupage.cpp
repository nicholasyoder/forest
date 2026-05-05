// SPDX-License-Identifier: LGPL-3.0-or-later

#include "menupage.h"

#include <QScrollBar>
#include <QDebug>

menupage::menupage(QList<menuitem*> items)
{
    foreach(menuitem *item, items){
        itemVLayout->addWidget(item);
        connect(item, &menuitem::focused, this, &menupage::itemFocused);
    }

    itemVLayout->setContentsMargins(QMargins(0,0,0,0));
    itemVLayout->setSpacing(0);
    itemVLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scrollWidget->setLayout(itemVLayout);
    scrollWidget->setObjectName("panelMainMenuScrollWidget");
    scrollArea->setObjectName("panelMainMenuListWidget");

    setLayout(baseVLayout);
    baseVLayout->addWidget(scrollArea);
    baseVLayout->setContentsMargins(QMargins(0,0,0,0));
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
