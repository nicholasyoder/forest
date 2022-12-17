#include "itemhandler.h"

#include <QScrollArea>
#include <QDrag>
#include <QMimeData>

itemhandler::itemhandler()
{
    settings->beginGroup("hitCounts");
    QStringList apps = settings->childKeys();
    foreach(QString app, apps){
        hitHash[app] = settings->value(app).toInt();
    }
    settings->endGroup();
}

menuitem* itemhandler::newItem(QString data, ITEM_TYPE type, int appPageIndex){
    menuitem *item = nullptr;
    switch (type) {
    case CATEGORY:
        item = newCategoryItem(data);
        leftPaneItems.append(item->uuid());
        break;
    case APP:
        item = newAppItem(data);
        rightPaneItems[appPageIndex].append(item->uuid());
        break;
    case SEARCH_APP:
        item = newAppItem(data, true);
        rightPaneItems[appPageIndex].append(item->uuid());
        break;
    case LEFT_PANE_APP:
        item = newAppItem(data);
        leftPaneItems.append(item->uuid());
        break;
    }
    return item;
}

menuitem* itemhandler::newCategoryItem(QString category){
    QUuid uuid = QUuid::createUuid();
    menuitem* item = new menuitem(uuid, categoryIcon(category), category);
    connect(item, &menuitem::activated, this, &itemhandler::itemEvent);

    itemHash[uuid] = item;
    catItems.append(uuid);

    return item;
}

menuitem* itemhandler::newAppItem(QString path, bool searchitem){
    XdgDesktopFile app;
    app.load(path);

    QUuid uuid = QUuid::createUuid();
    menuitem *item = new menuitem(uuid, app.icon(QIcon::fromTheme("application-x-executable")), app.name());
    connect(item, &menuitem::activated, this, &itemhandler::itemEvent);

    itemHash[uuid] = item;
    appHash[uuid] = app;

    if(searchitem)
        searchItems.append(uuid);
    else
        searchHash[app.name()] = path;

    return item;
}

void itemhandler::itemEvent(QUuid uuid, ITEM_EVENT eventtype, QEvent *event){
    if(eventtype == ENTER){
        if (allowSelectByMouse)
            currentItem = uuid;
    }
    else if(eventtype == LEAVE){
        if (allowSelectByMouse)
            currentItem = 0;
    }
    else if(eventtype == PRESS){

    }
    else if(eventtype == RELEASE){
        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
        if(mevent->button() == Qt::LeftButton)
            itemActivated(uuid);
        else if(mevent->button() == Qt::RightButton)
            showContextMenu(itemHash[uuid]->mapToGlobal(mevent->pos()));
    }
    else if(eventtype == DRAG){
        if(appHash.contains(currentItem))
            doDrag();
    }
}

void itemhandler::itemActivated(QUuid id){
    if(appHash.contains(id)){
        emit appLaunch();
        XdgDesktopFile dfile = appHash[id];
        incrementHits(dfile.name());
        dfile.startDetached();
    }
    else if(catItems.contains(id)){
        emit categorySelect(catItems.indexOf(id));
    }
}

void itemhandler::incrementHits(QString name){
    hitHash[name]++;
    QList<int> hitCountList = hitHash.values();

    QMap<int, int> hitcountmap;
    foreach(int hitCount, hitCountList){
        hitcountmap[hitCount] = 1;
    }

    QList<int> orderedHitCounts = hitcountmap.keys();

    settings->beginGroup("hitCounts");
    foreach(QString name, hitHash.keys()){
        int value = orderedHitCounts.indexOf(hitHash[name])+1;
        hitHash[name] = value;
        settings->setValue(name, value);
    }
    settings->endGroup();
}

void itemhandler::showContextMenu(QPoint pos){
    if(itemHash.contains(currentItem)){
        emit requestContextMenu(appHash[currentItem], pos);
    }
}

void itemhandler::doDrag(){
    QString dfilePath = appHash[currentItem].fileName();

    QDrag *drag = new QDrag(this);
    QMimeData *data = new QMimeData;
    data->setText("file://"+dfilePath);
    QList<QUrl> urls;
    urls << QUrl("file://"+dfilePath);
    data->setUrls(urls);
    drag->setMimeData(data);
    drag->exec();
}

void itemhandler::trySetAllowSelectByMouse(){
    if (SbyMtimer && SbyMtimer->isActive()){
        SbyMtimer->stop();
        delete SbyMtimer;
    }
    SbyMtimer = new QTimer();
    connect(SbyMtimer, &QTimer::timeout, this, &itemhandler::setAllowSelectByMouse);
    SbyMtimer->start(500);
}

void itemhandler::activateCurrentItem(){
    if(itemHash.contains(currentItem))
        itemActivated(currentItem);
}

void itemhandler::moveFocus(MOVE_DIRECTION direction){
    if (currentItem == 0){
        doInitialFocus(direction);
    }
    else if(direction == MOVE_RIGHT){
        if(catItems.contains(currentItem)){
            activateCurrentItem();
            setItemFocus(rightPaneItems[currentStackIndex].first());
        }
    }
    else if(direction == MOVE_LEFT){
        if(!catItems.contains(currentItem)){
            if(currentStackIndex < catItems.count())
                setItemFocus(catItems[currentStackIndex]);
            else
                setItemFocus(catItems.last());
        }
    }
    else {
        if (leftPaneItems.contains(currentItem))
            moveUpDown(direction, leftPaneItems);
        else
            moveUpDown(direction, rightPaneItems[currentStackIndex]);
    }
}

void itemhandler::doInitialFocus(MOVE_DIRECTION direction){
    if (direction == MOVE_UP)
        setItemFocus(leftPaneItems.last());
    else if (direction == MOVE_DOWN)
        setItemFocus(leftPaneItems.first());
}

void itemhandler::moveUpDown(MOVE_DIRECTION direction, QList<QUuid> items){
    int index = items.indexOf(currentItem);

    if(direction == MOVE_UP){
        index--;
        if (index < 0)
            setItemFocus(items.last());
        else
            setItemFocus(items[index]);
    }
    else {
        index++;
        if(index >= items.count())
            setItemFocus(items.first());
        else
            setItemFocus(items[index]);
    }
}

void itemhandler::setItemFocus(QUuid itemUuid){
    menuitem *olditem = itemHash[currentItem];
    if (olditem) olditem->setState(NORMAL);

    currentItem = itemUuid;

    menuitem *newitem = itemHash[currentItem];
    if (newitem) newitem->setState(FOCUS);
}

void itemhandler::setCategorySelected(QUuid itemUuid, bool searching){
    foreach(QUuid uuid, catItems){
        itemHash[uuid]->setSelected(false);
    }
    itemHash[itemUuid]->setSelected(true);
    if (searching)
        currentStackIndex = catItems.indexOf(itemUuid)+1;
    else
        currentStackIndex = catItems.indexOf(itemUuid);
}

QList<menuitem*> itemhandler::search(QString text){
    allowSelectByMouse = false;

    //select Everything cat button
    if (currentStackIndex != catItems.count())
        setCategorySelected(catItems.last(), true);

    foreach(QUuid id, searchItems){
        itemHash.remove(id);
        appHash.remove(id);
    }
    searchItems.clear();
    rightPaneItems[currentStackIndex].clear();

    QList<menuitem*> itemList;

    //only do an actual search if we have a search string
    if (text != ""){
        QStringList keys = searchHash.keys();//list of all app names matched to corresponding desktop file paths
        QStringList result = keys.filter(text, Qt::CaseInsensitive);//perform search

        QMap<int, QStringList> hitListMap;

        foreach (QString key, result){
            if(hitHash.contains(key))
                hitListMap[hitHash[key]].append(key);
            else
                hitListMap[0].append(key);
        }

        QList<int> hitcounts = hitListMap.keys();

        for(int i = hitcounts.length()-1; i >= 0; i--){
            QStringList hitList = hitListMap[hitcounts[i]];
            hitList.sort(Qt::CaseInsensitive);
            foreach (QString key, hitList){
                menuitem *item = newItem(searchHash[key], SEARCH_APP, currentStackIndex);
                itemList.append(item);
                item->setState(NORMAL);
            }
        }

        if(itemList.count())
            setItemFocus(itemList.first()->uuid());
        else
            setItemFocus(0);
    }
    else {
        setCategorySelected(catItems.last());
        setItemFocus(rightPaneItems[catItems.count()-1].first());
    }

    trySetAllowSelectByMouse();
    return itemList;
}

void itemhandler::clear(){
    currentItem = 0;
    currentStackIndex = 0;
    itemHash.clear();
    appHash.clear();
    catItems.clear();
    searchHash.clear();
    searchItems.clear();
    leftPaneItems.clear();
    rightPaneItems.clear();
}

QIcon itemhandler::categoryIcon(QString category){
    if (category == "Network")
        return XdgIcon::fromTheme("applications-internet");
    else if (category == "Utility")
        return XdgIcon::fromTheme("applications-accessories");
    else if (category == "Settings")
        return XdgIcon::fromTheme("preferences-system");
    else if (category == "Everything")
        return XdgIcon::fromTheme("applications-other");
    else
        return XdgIcon::fromTheme("applications-" + category.toLower());
}
