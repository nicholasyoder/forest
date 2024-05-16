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

    Sensors mSensors;
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
