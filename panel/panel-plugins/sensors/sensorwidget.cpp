// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sensorwidget.h"
#include "sensorsconfig.h"
#include <QSettings>
#include <QGenericPlugin>

SensorWidget::SensorWidget(){
}

void SensorWidget::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    setText(QChar(0x00B0)+QString::number(100));

    mSensors = new Sensors;
    mDetectedChips = mSensors->getDetectedChips();
    layoutdirection = layout->direction();
    pbutton = new panelbutton;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(QMargins(0,0,0,0));
    hlayout->addWidget(this);
    pbutton->setLayout(hlayout);
    layout->addWidget(pbutton);

    mLabelInfo=new QLabel;
    mLabelInfo->setAlignment(Qt::AlignCenter);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(mLabelInfo);
    popupbox = new popup(vlayout, pbutton, CenteredOnWidget);
    connect(pbutton, &panelbutton::leftclicked, popupbox, &popup::showpopup);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);
    connect(pbutton, &panelbutton::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, pbutton, CenteredOnWidget); });

    timer=new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(updateSensor()));

    loadSettings();
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
    namespace cfg = sensorsconfig;
    timer->stop();

    QSettings settings("Forest", "Temperature Monitor");
    settings.sync();
    mTimeUpdat = settings.value(cfg::updateinterval, cfg::updateinterval_default).toInt()*1000;
    mFahrenheit = settings.value(cfg::fahrenheit, cfg::fahrenheit_default).toBool();
    shownsensor = settings.value(cfg::sensor).toString();
    displaytype = settings.value(cfg::display, cfg::display_text).toString();
    warningtemp = settings.value(cfg::warningtemp, cfg::warningtemp_default).toInt();
    criticaltemp = settings.value(cfg::criticaltemp, cfg::criticaltemp_default).toInt();
    hiddenbars = settings.value(cfg::hiddenbars).toStringList();

    barwidth = settings.value(cfg::barwidth, cfg::barwidth_default).toInt();
    barspacing = settings.value(cfg::barspacing, cfg::barspacing_default).toInt();
    margin = settings.value(cfg::margin, cfg::margin_default).toInt();
    maxtemp = settings.value(cfg::maxtemp, cfg::maxtemp_default).toInt();
    backcolor = settings.value(cfg::backcolor, cfg::backcolor_default).value<QColor>();

    updateSensor();

    timer->start(mTimeUpdat);
}

void SensorWidget::updateSensor(){
    temps.clear();
    popuptext = "<table cellspacing='0' cellpadding='3'>";

    int index=-1;
    double curTemp = 0;
    bool shownfound = false;

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

                QString id = sensorsconfig::sensor_id(mDetectedChips[i], features[j]);
                if (!hiddenbars.contains(id))
                    temps.append(int(curTemp));

                // First sensor until the chosen one turns up.
                if (index == 0 || (!shownfound && id == shownsensor)){
                    shownfound = id == shownsensor;
                    iconTemp = mFahrenheit ? celsius2fahrenheit(curTemp) : curTemp;
                    cicontemp = curTemp;
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
