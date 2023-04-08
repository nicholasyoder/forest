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

#ifndef MAINMENU_H
#define MAINMENU_H

#include <QStackedLayout>
#include <QLineEdit>

#include "panelpluginterface.h"
#include "popup.h"
#include "panelbutton.h"
#include "menuitem.h"
#include "menupage.h"

#include "apploader.h"
#include "itemhandler.h"
#include "contextmenu.h"

class mainmenu : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.mainmenu.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    mainmenu();
    ~mainmenu();

    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();

public slots:
    void togglemenu();
    void showmenu();
    void closemenu();

private slots:
    void loadSettings();
    void loadUI();
    void reloadMenu(QMap<QString, QStringList> menuHash);
    void loadMenu(QMap<QString, QStringList> menuHash);
    void search(QString text);
    void handleKeyPressed(QKeyEvent *event);
    void setCategory(int index);
    void setAllowClose(){allowClose = true;}

private:
    QSettings *settings = new QSettings("Forest","Main Menu");

    popup *pBox = nullptr;
    popupmenu *pMenu = nullptr;
    contextmenu *cMenu = nullptr;
    QStackedLayout *pagesLayout = new QStackedLayout;
    QVBoxLayout *categoriesVLayout = new QVBoxLayout;
    QLineEdit *searchBox = new QLineEdit;

    apploader *appLdr = nullptr;
    QHash<QUuid, int> catIndexList;

    bool allowClose = false;//nasty hack to keep Ctrl+Esc from making the menu go away as soon as it's shown
    QUuid currentitem = 0;
    QUuid poweritem = 0;

    itemhandler *itemHandler = nullptr;
};

#endif // MAINMENU_H
