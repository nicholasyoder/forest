/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
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

#include "geometrymanager.h"

#include <QApplication>
#include <QScreen>

#include "xcbutills.h"

GeometryManager::GeometryManager(QWidget *panel) : panel_widget(panel) {
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &GeometryManager::update_geometry);
}

GeometryManager::~GeometryManager(){
    delete panel_widget;
    panel_widget = nullptr;
}

void GeometryManager::set_fixed_size(int size){
    fixed_panel_size = size;
}

void GeometryManager::set_panel_position(QString position){
    panel_position = position;
}

void GeometryManager::set_reserve_screen_space(bool reserve){
    reserve_screen_space = reserve;
}

void GeometryManager::update_geometry(){

    // reset fixed size
    panel_widget->setMinimumSize(0,0);
    panel_widget->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

    // default to sizeHint when no fixed size is set
    if(fixed_panel_size == 0)
        fixed_panel_size = panel_widget->sizeHint().height();

    QSize scsize = qApp->primaryScreen()->size();
    if (panel_position == "top"){
        panel_widget->move(0,0);
        panel_widget->setFixedSize(scsize.width(), fixed_panel_size);
        if(reserve_screen_space)
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,panel_widget->height(),0,0,0,0,0,panel_widget->geometry().left(),panel_widget->geometry().right(),0,0);
        else
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,0,0,0,0,0,0,0,0,0,0);
    /*} else if (panel_position == "left"){
        panel_widget->move(0,0);
        panel_widget->setFixedSize(fixed_panel_size, scsize.height());
        //Xcbutills::setPartialStrut(winId(),height(),0,0,0,geometry().top(),geometry().bottom(),0,0,0,0,0,0);
    } else if (panel_position == "right"){
        panel_widget->move(scsize.width() - fixed_panel_size,0);
        panel_widget->setFixedSize(fixed_panel_size, scsize.height());
        //Xcbutills::setPartialStrut(winId(),0,height(),0,0,0,0,geometry().top(),geometry().bottom(),0,0,0,0);*/
    } else{//bottom
        panel_widget->move(0,scsize.height() - fixed_panel_size);
        panel_widget->setFixedSize(scsize.width(), fixed_panel_size);
        if(reserve_screen_space)
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,0,panel_widget->height(),0,0,0,0,0,0,panel_widget->geometry().left(),panel_widget->geometry().right());
        else
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,0,0,0,0,0,0,0,0,0,0);
    }
}
