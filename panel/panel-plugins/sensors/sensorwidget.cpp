// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sensorwidget.h"
#include "sensorsconfig.h"
#include <QSettings>
#include <QGenericPlugin>

SensorWidget::SensorWidget(){
}

void SensorWidget::setupPlug(QBoxLayout *layout, QList<QAction*> itemlist){
    setText(QString());

    mSensors = new Sensors;
    mDetectedChips = mSensors->getDetectedChips();
    // Bars run across the panel: vertical on a horizontal panel.
    bool vertical_panel = layout->direction() == QBoxLayout::TopToBottom;
    barorientation = vertical_panel ? Qt::Horizontal : Qt::Vertical;
    pbutton = new panelbutton;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(QMargins(0,0,0,0));
    hlayout->addWidget(this);

    bars = new QFrame;
    bars->setObjectName("sensorBars");
    barlayout = new QBoxLayout(vertical_panel ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight, bars);
    if (vertical_panel) bars->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    else bars->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    hlayout->addWidget(bars);
    pbutton->setLayout(hlayout);
    layout->addWidget(pbutton);

    mLabelInfo=new QLabel;
    mLabelInfo->setAlignment(Qt::AlignCenter);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(mLabelInfo);
    popupbox = new popup(vlayout, pbutton, CenteredOnWidget);
    connect(pbutton, &panelbutton::leftclicked, this, &SensorWidget::showpopupnow);

    pmenu = new QMenu;
    pmenu->addActions(itemlist);
    connect(pbutton, &panelbutton::rightclicked, this, [this]{ popupMenuOnLauncher(pmenu, pbutton, CenteredOnWidget); });

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(updateSensor()));

    loadSettings();
}

void SensorWidget::paintEvent(QPaintEvent *){
    QPainter painter(this);
    QPen pen(palette().color(QPalette::WindowText));
    if (iconTemp >= criticaltemp)
        pen.setColor(Qt::red);
    else if (iconTemp >= warningtemp)
        pen.setColor(Qt::yellow);

    painter.setPen(pen);
    painter.drawText(0,0,this->width(),this->height(), Qt::AlignCenter, this->text());
}

void SensorWidget::loadSettings(){
    namespace cfg = sensorsconfig;
    timer->stop();

    QSettings settings("Forest", "Temperature Monitor");
    settings.sync();
    mTimeUpdat = qMax(1, settings.value(cfg::updateinterval, cfg::updateinterval_default).toInt()) * 1000;
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

    bool showbars = displaytype == cfg::display_bars;
    setVisible(!showbars);
    bars->setVisible(showbars);
    bars->setStyleSheet(QString("#sensorBars { background: %1; }").arg(backcolor.name(QColor::HexArgb)));
    barlayout->setSpacing(barspacing);
    if (barorientation == Qt::Vertical) barlayout->setContentsMargins(margin, 0, margin, 0);
    else barlayout->setContentsMargins(0, margin, 0, margin);
    qDeleteAll(barlist); // rebuilt by updateSensor() with the new width
    barlist.clear();

    updateSensor();

    timer->start(mTimeUpdat);
}

QString SensorWidget::formatTemp(double celsius){
    return mFahrenheit ? QString::number(int(celsius2fahrenheit(celsius))) + QChar(0x00B0) + "F"
                       : QString::number(int(celsius)) + QChar(0x00B0) + "C";
}

void SensorWidget::showpopupnow(){
    buildpopup = true;
    updateSensor();
    buildpopup = false;
    popupbox->showpopup();
}

void SensorWidget::updateSensor(){
    temps.clear();
    // Values are only read for what's displayed; the popup lists every sensor.
    const bool popupshown = buildpopup || popupbox->isVisible();
    QString popuptext = "<table cellspacing='0' cellpadding='3'>";

    int index=-1;
    bool shownfound = false;
    bool anysensor = false;

    for (unsigned int i = 0; i < mDetectedChips.size(); ++i){
        const std::vector<Feature>& features = mDetectedChips[i].getFeatures();
        for (unsigned int j = 0; j < features.size(); ++j){
            if (features[j].getType() != SENSORS_FEATURE_TEMP)
                continue;
            index++;
            anysensor = true;
            QString id = sensorsconfig::sensor_id(mDetectedChips[i], features[j]);
            const bool wantbar = displaytype == sensorsconfig::display_bars && !hiddenbars.contains(id);
            const bool wanticon = index == 0 || (!shownfound && id == shownsensor);
            if (!popupshown && !wantbar && !wanticon)
                continue;

            double curTemp = features[j].getValue(SENSORS_SUBFEATURE_TEMP_INPUT);

            if (popupshown){
                QString border = (index != 0) ? "style='border-top: 1px solid #aaa;'" : "";
                popuptext += "<tr><td "+border+">"+QString::fromStdString(features[j].getLabel())+"</td><td "+border+">&nbsp;&nbsp;&nbsp;</td>"
                             "<td "+border+">"+formatTemp(curTemp)+"</td></tr>";
            }
            if (wantbar)
                temps.append(int(curTemp));

            // First sensor until the chosen one turns up.
            if (wanticon){
                shownfound = id == shownsensor;
                iconTemp = curTemp;
            }
        }
    }
    popuptext += "</table>";

    if (popupshown)
        mLabelInfo->setText(anysensor ? popuptext : "No temperature sensors");

    if (displaytype == sensorsconfig::display_bars){
        if (barlist.size() != temps.size()){
            qDeleteAll(barlist);
            barlist.clear();
            for (int i = 0; i < temps.size(); ++i){
                SensorBar *bar = new SensorBar(barorientation);
                if (barorientation == Qt::Vertical) bar->setFixedWidth(barwidth);
                else bar->setFixedHeight(barwidth);
                barlayout->addWidget(bar);
                barlist.append(bar);
            }
        }
        for (int i = 0; i < temps.size(); ++i)
            barlist[i]->setValue(double(temps[i]) / maxtemp);
    }
    else{
        setText(anysensor ? formatTemp(iconTemp) : "N/A");
        setFixedWidth(sizeHint().width());
        update();
    }
}

double SensorWidget::celsius2fahrenheit(double celsius){
    return 32 + 1.8 * celsius;
}
