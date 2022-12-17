#ifndef ITEMHANDLER_H
#define ITEMHANDLER_H

#include <QObject>
#include <QTimer>

#include "menuitem.h"

#include <QDebug>

enum MOVE_DIRECTION { MOVE_UP, MOVE_DOWN, MOVE_RIGHT, MOVE_LEFT };
enum ITEM_TYPE { CATEGORY, APP, SEARCH_APP, LEFT_PANE_APP };

class itemhandler : public QObject
{
    Q_OBJECT

public:
    itemhandler();

public slots:
    menuitem* newItem(QString data, ITEM_TYPE type = CATEGORY, int appPageIndex = 0);
    menuitem* newCategoryItem(QString category);
    menuitem* newAppItem(QString path, bool searchitem = false);
    menuitem* item(QUuid uuid){return itemHash[uuid];}

    void activateCurrentItem();
    void moveFocus(MOVE_DIRECTION direction);
    void doInitialFocus(MOVE_DIRECTION direction);
    void moveUpDown(MOVE_DIRECTION direction, QList<QUuid> items);
    void setItemFocus(QUuid itemUuid);
    void setCategorySelected(QUuid itemUuid, bool searching = false);
    QList<menuitem *> search(QString text);
    void clear();
    void setAllowSelectByMouse(){allowSelectByMouse = true;}
    void setSelectByMouse(bool allow){allowSelectByMouse = allow;}

signals:
    void appLaunch();
    void categorySelect(int index);
    void requestContextMenu(XdgDesktopFile deskfile, QPoint pos);

private slots:
    void itemEvent(QUuid uuid, ITEM_EVENT eventtype, QEvent *event);
    void itemActivated(QUuid id);
    void incrementHits(QString name);
    void showContextMenu(QPoint pos);
    void doDrag();
    void trySetAllowSelectByMouse();


private:
    QIcon categoryIcon(QString category);

    QUuid currentItem = 0;
    int currentStackIndex = 0;

    QHash<QUuid, menuitem*> itemHash;
    QHash<QUuid, XdgDesktopFile> appHash;
    QList<QUuid> catItems;
    QHash<QString, QString> searchHash;
    QList<QUuid> searchItems;
    QHash<QString, int> hitHash;

    QList<QUuid> leftPaneItems;
    QHash<int, QList<QUuid>> rightPaneItems;

    QSettings *settings = new QSettings("Forest","Main Menu");
    bool allowSelectByMouse = true;
    QTimer *SbyMtimer = nullptr;

};

#endif // ITEMHANDLER_H
