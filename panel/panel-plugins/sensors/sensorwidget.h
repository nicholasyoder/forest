// SPDX-License-Identifier: LGPL-3.0-or-later

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
#include "panelbutton.h"
#include "sensorbar.h"

class SensorWidget : public QLabel, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.sensors.plugin" FILE "sensors.json")
    Q_INTERFACES(panelpluginterface)

public:
    SensorWidget();
    ~SensorWidget() override { delete pmenu; delete popupbox; delete mSensors; }

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void reloadSettings(){ loadSettings(); }
    //end plugininterface

private slots:
    void paintEvent(QPaintEvent *);
    void updateSensor();
    void loadSettings();
    void showpopupnow();
    double celsius2fahrenheit(double celsius);

private:
    QString formatTemp(double celsius);
    bool buildpopup = false;

private:
    panelbutton *pbutton;
    QLabel *mLabelInfo;
    QFrame *bars;
    QBoxLayout *barlayout;
    QList<SensorBar*> barlist;
    popup *popupbox = nullptr;
    QMenu *pmenu = nullptr;

    Sensors *mSensors = nullptr;
    std::vector<Chip> mDetectedChips;

    bool mFahrenheit;
    int  mTimeUpdat;
    QString shownsensor;
    double iconTemp = 0;
    QString displaytype;
    int warningtemp = 0;
    int criticaltemp = 0;
    int barwidth = 0;
    int barspacing = 0;
    int margin = 0;
    int maxtemp = 0;
    QTimer *timer;
    QList<int> temps;
    QStringList hiddenbars;
    QColor backcolor;
    Qt::Orientation barorientation;

};

#endif // SENSORWIDGET_H
