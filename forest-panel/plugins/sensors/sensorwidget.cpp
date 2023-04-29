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

#include "sensorwidget.h"
#include "widgetsensorconf.h"
#include <QSettings>
#include <QGenericPlugin>

SensorWidget::SensorWidget(){
}

void SensorWidget::setupPlug(QBoxLayout *layout, QList<pmenuitem *> itemlist){
    setText(QChar(0x00B0)+QString::number(100));

    mDetectedChips = mSensors.getDetectedChips();
    int sensornum = 0;
    for (unsigned int i = 0; i < mDetectedChips.size(); ++i){
        const std::vector<Feature>& features = mDetectedChips[i].getFeatures();
        for (unsigned int j = 0; j < features.size(); ++j){
            if (features[j].getType() == SENSORS_FEATURE_TEMP)
                sensornum++;
        }
    }

    layoutdirection = layout->direction();
    pbutton = new panelbutton;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->addWidget(this);
    pbutton->setLayout(hlayout);
    layout->addWidget(pbutton);

    mLabelInfo=new QLabel;
    mLabelInfo->setForegroundRole(QPalette::Light);
    mLabelInfo->setAlignment(Qt::AlignCenter);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(mLabelInfo);
    popupbox = new popup(vlayout, pbutton, CenteredOnWidget);
    connect(pbutton, &panelbutton::leftclicked, popupbox, &popup::showpopup);

    pmenu = new popupmenu(pbutton, CenteredOnWidget);
    foreach (pmenuitem *item, itemlist)
        pmenu->additem(item);

    pmenu->addseperator();
    pmenuitem *item = new pmenuitem("Sensors Settings", QIcon::fromTheme("configure"));
    connect(item, &pmenuitem::clicked, this, &SensorWidget::showsettingswidget);
    pmenu->additem(item);
    connect(pbutton, &panelbutton::rightclicked, pmenu, &popupmenu::show);

    timer=new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(updateSensor()));
    this->setForegroundRole(QPalette::Light);

    loadSettings();
}

QHash<QString, QString> SensorWidget::getpluginfo(){
    QHash<QString, QString> info;
    info["name"] = "Sensor Monitor";
    return info;
}

void SensorWidget::paintEvent(QPaintEvent *){
    if (displaytype == "bars"){
        if (layoutdirection == QBoxLayout::TopToBottom){
            int displayheight = this->width();

            QPainter painter1(this);
            painter1.fillRect(0, 0, displayheight, this->height(), backcolor);

            int y = margin;
            foreach (int temp, temps){
                double percentage = double(temp) / maxtemp;
                double barheight = double(displayheight) * percentage;

                QLinearGradient gradient(displayheight, 0, 0, 0);
                gradient.setColorAt(0.2, Qt::red);
                gradient.setColorAt(0.5, Qt::yellow);
                gradient.setColorAt(1, Qt::green);

                QBrush brush(gradient);
                QPainter painter(this);
                painter.fillRect(0, y, int(barheight), barwidth, brush);
                y += barwidth + barspacing;
            }
        }
        else{
            int displayheight = this->height();

            QPainter painter1(this);
            painter1.fillRect(0, 0, this->width(), displayheight, backcolor);

            int x = margin;
            foreach (int temp, temps)
            {
                double percentage = double(temp) / maxtemp;
                double barheight = double(displayheight) * percentage;

                QLinearGradient gradient(0, 0, 0, displayheight);
                gradient.setColorAt(0.2, Qt::red);
                gradient.setColorAt(0.5, Qt::yellow);
                gradient.setColorAt(1, Qt::green);

                QBrush brush(gradient);
                QPainter painter(this);
                painter.fillRect(x, displayheight - int(barheight), barwidth, int(barheight), brush);
                x += barwidth + barspacing;
            }
        }
    }
    else{
        QPainter painter(this);
        QPen pen;
        if (cicontemp < warningtemp)
            pen.setColor(Qt::green);
        else if(cicontemp < criticaltemp)
            pen.setColor(Qt::yellow);
        else if(cicontemp >= criticaltemp)
            pen.setColor(Qt::red);

        painter.setPen(pen);
        painter.drawText(0,0,this->width(),this->height(), Qt::AlignCenter, this->text());
    }
}

void SensorWidget::loadSettings(){
    timer->stop();

    QSettings settings("Forest", "Temperature Monitor");
    settings.sync();
    mTimeUpdat = settings.value("TimeUpdat",3).toInt()*1000;
    mFahrenheit = settings.value("Fahrenheit",false).toBool();
    mChipIndex = settings.value("ChipIndex",0).toInt();
    displaytype = settings.value("display", "text").toString();
    warningtemp = settings.value("warningtemp", 70).toInt();
    criticaltemp = settings.value("criticaltemp", 90).toInt();
    QStringList sensorlist = settings.value("sensorlist").toStringList();
    foreach (QString sensor, sensorlist)
        enabledbars[sensor] = settings.value(sensor + "Barenabled", true).toBool();

    barwidth = settings.value("barwidth", 5).toInt();
    barspacing = settings.value("barspacing", 2).toInt();
    margin = settings.value("margin", 2).toInt();
    maxtemp = settings.value("maxtemp", 110).toInt();
    backcolor = settings.value("backcolor").value<QColor>();

    updateSensor();

    timer->start(mTimeUpdat);
}

void SensorWidget::showsettingswidget(){
    QStringList list;
    for (unsigned int i = 0; i < mDetectedChips.size(); ++i){
        const std::vector<Feature>& features = mDetectedChips[i].getFeatures();
        for (unsigned int j = 0; j < features.size(); ++j){
            if (features[j].getType() == SENSORS_FEATURE_TEMP){
                QString name= QString::fromStdString(features[j].getLabel());
                list.append(name);
            }
        }
    }
    WidgetSensorConf *dlg=new WidgetSensorConf(list);
    connect(dlg,SIGNAL(settingSaved()),this,SLOT(loadSettings()));
    dlg->show();
}

void SensorWidget::updateSensor(){
    temps.clear();
    popuptext = "<table cellspacing='0' cellpadding='3'>";

    int index=-1;
    double curTemp = 0;

    for (unsigned int i = 0; i < mDetectedChips.size(); ++i){
        const std::vector<Feature>& features = mDetectedChips[i].getFeatures();
        for (unsigned int j = 0; j < features.size(); ++j){
            if (features[j].getType() == SENSORS_FEATURE_TEMP){
                index++;
                QString name= QString::fromStdString(features[j].getLabel());

                QString border = (index != 0) ? "style='border-top: 1px solid #aaa;'" : "";

                popuptext += "<tr><td "+border+">"+name+"</td><td "+border+">&nbsp;&nbsp;&nbsp;</td>";

                curTemp = features[j].getValue(SENSORS_SUBFEATURE_TEMP_INPUT);

                popuptext += "<td "+border+">";
                if (mFahrenheit) popuptext += QString::number(int(celsius2fahrenheit(curTemp))) + QChar(0x00B0)+"F";
                else popuptext += QString::number(int(curTemp)) +" C"+ QChar(0x00B0);
                popuptext += "</td>";

                if (enabledbars[name] == true || enabledbars.isEmpty())
                    temps.append(int(curTemp));

                if(mChipIndex==index){
                    if (mFahrenheit){
                        iconTemp = celsius2fahrenheit(curTemp);
                        cicontemp = curTemp;
                    }
                    else{
                        iconTemp = curTemp;
                        cicontemp = curTemp;
                    }
                }

                popuptext += "</tr>";
            }
        }
    }
    popuptext += "</table>";

    mLabelInfo->setText(popuptext);

    if (displaytype == "text"){
        if(mFahrenheit)
            setText(QString::number(int(iconTemp)) +"F"+ QChar(0x00B0));
        else
            setText(QString::number(int(iconTemp)) +"C"+ QChar(0x00B0));
        setFixedWidth(sizeHint().width());
    }
    else{
        setText("");
        int size = temps.length() * (barwidth + barspacing) + margin*2 - barspacing;
        if (layoutdirection == QBoxLayout::TopToBottom)
            setFixedHeight(size);
        else
            setFixedWidth(size);
        update();
    }
}

double SensorWidget::celsius2fahrenheit(double celsius){
    return 32 + 1.8 * celsius;
}
