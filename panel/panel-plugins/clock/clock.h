#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QCalendarWidget>
#include <QTimer>
#include <QDebug>
#include <QtDBus>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "popupmenu.h"

class clockplug : public panelbutton, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.clock.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    clockplug();
    ~clockplug();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugininterface

private slots:
    void loadsettings();
    void updatetime();
    void showsettingswidget();
    void showpopup(){popupbox->showpopup();}

private:
    bool twelvehour = false;
    bool showseconds = false;
    QString time_format = "h:mm";
    QDate currentDate;
    QCalendarWidget *cwidget = new QCalendarWidget;
    popup *popupbox;
    popupmenu *pmenu;
};

#endif // CLOCK_H
