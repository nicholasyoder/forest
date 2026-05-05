// SPDX-License-Identifier: LGPL-3.0-or-later

﻿
#ifndef SENSORWIDGET_H
#define SENSORWIDGET_H

#include "panelpluginterface.h"
#include "sensor/sensors.h"
//#include "../../../common/widgetpopup/widgetpopup.h"
#include <QLabel>
#include <QTimer>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>
#include <QtDBus>
#include <QColor>

#include "popup.h"
#include "popupmenu.h"
#include "panelbutton.h"

class SensorWidget : public QLabel, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.sensors.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    SensorWidget();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void paintEvent(QPaintEvent *);
    void updateSensor();
    void loadSettings();
    void showsettingswidget();
    double celsius2fahrenheit(double celsius);

private:
    panelbutton *pbutton;
    QLabel *mLabelInfo;
    QString popuptext;
    popup *popupbox;
    popupmenu *pmenu;

    Sensors *mSensors = nullptr;
    std::vector<Chip> mDetectedChips;

    bool mFahrenheit;
    int  mTimeUpdat;
    int mChipIndex;
    double iconTemp = 0;
    double cicontemp = 0;
    QString displaytype;
    //int displayheight = 0;
    int warningtemp = 0;
    int criticaltemp = 0;
    int barwidth = 0;
    int barspacing = 0;
    int margin = 0;
    int maxtemp = 0;
    QTimer *timer;
    QList<int> temps;
    QHash<QString, bool> enabledbars;
    QColor backcolor;
    QBoxLayout::Direction layoutdirection;
    QString plugnum;

};

#endif // SENSORWIDGET_H
