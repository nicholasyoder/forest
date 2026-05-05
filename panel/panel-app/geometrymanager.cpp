// SPDX-License-Identifier: LGPL-3.0-or-later

#include "geometrymanager.h"

#include <QApplication>
#include <QScreen>

#include "xcbutills.h"
#include "miscutills.h"

GeometryManager::GeometryManager(QWidget *panel) : panel_widget(panel) {
    RunOnce* runner = new RunOnce(2000);
    connect(qApp, &QGuiApplication::screenAdded, runner, &RunOnce::try_activate);
    connect(qApp, &QGuiApplication::screenRemoved, runner, &RunOnce::try_activate);
    connect(qApp->primaryScreen(), &QScreen::geometryChanged, runner, &RunOnce::try_activate);
    connect(runner, &RunOnce::activated, this, &GeometryManager::update_geometry);
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

    QRect sc_geo = qApp->primaryScreen()->geometry();
    if (panel_position == "top"){
        panel_widget->move(sc_geo.left(), 0);
        panel_widget->setFixedSize(sc_geo.width(), fixed_panel_size);
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
        panel_widget->move(sc_geo.left(), sc_geo.height() - fixed_panel_size);
        panel_widget->setFixedSize(sc_geo.width(), fixed_panel_size);
        if(reserve_screen_space)
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,0,panel_widget->height(),0,0,0,0,0,0,panel_widget->geometry().left(),panel_widget->geometry().right());
        else
            Xcbutills::setPartialStrut(panel_widget->winId(),0,0,0,0,0,0,0,0,0,0,0,0);
    }
}
