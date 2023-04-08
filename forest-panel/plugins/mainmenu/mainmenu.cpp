/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2022 Nicholas Yoder
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

#include "mainmenu.h"

#include <QDBusConnection>
#include <QTimer>

mainmenu::mainmenu(){

}

mainmenu::~mainmenu(){

}

void mainmenu::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    layout->addWidget(this);

    itemHandler = new itemhandler();
    connect(itemHandler, &itemhandler::categorySelect, this, &mainmenu::setCategory);
    connect(itemHandler, &itemhandler::appLaunch, this, &mainmenu::closemenu);

    cMenu = new contextmenu();
    connect(itemHandler, &itemhandler::requestContextMenu, cMenu, &contextmenu::show);

    loadSettings();
    loadUI();

    QStringList watchedDirs;
    watchedDirs << "/usr/share/applications/" << QDir::home().absolutePath() + "/.local/share/applications/";
    appLdr = new apploader(watchedDirs);
    connect(appLdr, &apploader::appLoad, this, &mainmenu::reloadMenu);
    appLdr->load();

    //popup menu~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    pMenu = new popupmenu(this, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pMenu->additem(item);

    //pmenu->addseperator();
    //pmenuitem *item = new pmenuitem("Menu Settings", QIcon::fromTheme("configure"));
    //connect(item, &pmenuitem::clicked, this, &mainmenu::showsettingswidget);
    //pmenu->additem(item);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    connect(this, &mainmenu::leftclicked, this, &mainmenu::showmenu);
    connect(this, &mainmenu::rightclicked, pMenu, &popupmenu::show);

    QDBusConnection::sessionBus().registerObject("/org/forest/panel/mainmenu", this, QDBusConnection::ExportAllSlots);
}

QHash<QString, QString> mainmenu::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Mainmenu";
    return info;
}

void mainmenu::togglemenu(){
    if(pBox->isVisible())
        closemenu();
    else
        showmenu();
}

void mainmenu::showmenu(){
    allowClose = false;
    searchBox->setText("");
    itemHandler->setItemFocus(0);
    itemHandler->setSelectByMouse(false);
    setCategory(catIndexList.count()-1);
    pBox->showpopup();
    QTimer::singleShot(100, this, SLOT(setAllowClose()));
    QTimer::singleShot(100, itemHandler, SLOT(setAllowSelectByMouse()));
}

void mainmenu::closemenu(){
    if(!allowClose)
        return;

    pBox->closepopup();
}

void mainmenu::loadSettings(){
    settings->sync();
    setupIconButton(settings->value("menuicon", "/usr/share/forest/pics/logo-small.png").toString());
}

void mainmenu::loadUI(){
    QHBoxLayout *mainHLayout = new QHBoxLayout;
    mainHLayout->setMargin(0);
    mainHLayout->setSpacing(0);
    mainHLayout->addLayout(categoriesVLayout, 2);
    categoriesVLayout->setSpacing(0);

    QVBoxLayout *rightVLayout = new QVBoxLayout;
    rightVLayout->setMargin(0);
    rightVLayout->setSpacing(0);
    mainHLayout->addLayout(rightVLayout, 5);
    rightVLayout->addLayout(pagesLayout);
    rightVLayout->addWidget(searchBox);

    searchBox->setPlaceholderText("Search...");
    searchBox->setObjectName("panelMainmenuSearchbox");
    connect(searchBox, SIGNAL(textChanged(QString)), this, SLOT(search(QString)));

    pBox = new popup(mainHLayout, this, CenteredOnWidget);
    pBox->setObjectName("panelMainMenuPopup");
    //pBox->setFixedSize(500,500);
    connect(pBox, &popup::keypressed, this, &mainmenu::handleKeyPressed);
}

void mainmenu::reloadMenu(QMap<QString, QStringList> menuHash){
    QLayoutItem *child;
    while ((child = categoriesVLayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }
    while ((child = pagesLayout->takeAt(0)) != nullptr){
        delete child->widget();
        delete child;
    }

    catIndexList.clear();
    itemHandler->clear();

    loadMenu(menuHash);
}

void mainmenu::loadMenu(QMap<QString, QStringList> menuHash){
    QStringList cats = menuHash.keys();
    cats.removeOne("Everything");
    cats.append("Everything");
    int stackIndex = 0;
    foreach(QString category, cats){
        menuitem *categoryItem = itemHandler->newItem(category);
        QUuid catUuid = categoryItem->uuid();
        QList<menuitem*> itemList;
        menuHash[category].sort(Qt::CaseInsensitive);

        foreach(QString id, menuHash[category]){
            QString path = id.split(",").last();

            menuitem *appItem = itemHandler->newItem(path, APP, stackIndex);
            itemList.append(appItem);

        }
        menupage *categoryPage = new menupage(itemList);

        categoriesVLayout->addWidget(categoryItem);
        pagesLayout->addWidget(categoryPage);
        catIndexList[catUuid] = stackIndex;

        stackIndex++;
    }
    categoriesVLayout->addStretch(1);

    menuitem *pitem = itemHandler->newItem("/usr/share/forest/power-manager.desktop", LEFT_PANE_APP);
    categoriesVLayout->addWidget(pitem);

    pagesLayout->addWidget(new QWidget());
}

void mainmenu::search(QString text){
    //delete old search page
    int lastpage = pagesLayout->count()-1;
    if(pagesLayout->count() > catIndexList.count()){
        QLayoutItem *child;
        child = pagesLayout->takeAt(lastpage);
        delete child->widget();
        delete child;
    }

    QList<menuitem*> items = itemHandler->search(text);

    if(text != ""){
        menupage *searchPage = new menupage(items);
        pagesLayout->addWidget(searchPage);
    }
    else {
        searchBox->clearFocus();
    }

    //this will hit the seach page or the Everything page if no search text
    pagesLayout->setCurrentIndex(pagesLayout->count()-1);
}

void mainmenu::handleKeyPressed(QKeyEvent *event)
{
    int eventkey = event->key();

    switch (eventkey) {
    case Qt::Key_AltGr: return; break;
    case Qt::Key_Print: return; break;
    case Qt::Key_CapsLock: return; break;
    case Qt::Key_NumLock: return; break;
    case Qt::Key_Backspace:{
            QString text = searchBox->text();
            if (text != ""){
                text.chop(1);
                searchBox->setText(text);
                searchBox->setFocus();
            }
        }
        break;
    case Qt::Key_Return: itemHandler->activateCurrentItem(); break;
    case Qt::Key_Enter: itemHandler->activateCurrentItem(); break;
    case Qt::Key_Shift: return; break;
    case Qt::Key_Tab: return; break;
    case Qt::Key_Up: itemHandler->moveFocus(MOVE_UP); searchBox->clearFocus(); break;
    case Qt::Key_Down: itemHandler->moveFocus(MOVE_DOWN); searchBox->clearFocus(); break;
    case Qt::Key_Left: itemHandler->moveFocus(MOVE_LEFT); break;
    case Qt::Key_Right: itemHandler->moveFocus(MOVE_RIGHT); break;
    case Qt::Key_Escape: closemenu(); break;
    default: {
            searchBox->setText(event->text());
            searchBox->setFocus();
        }
        break;
    }
}

void mainmenu::setCategory(int index){
    pagesLayout->setCurrentIndex(index);
    itemHandler->setCategorySelected(catIndexList.key(index));
}
