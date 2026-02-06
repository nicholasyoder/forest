#ifndef DESKSWITCH_H
#define DESKSWITCH_H

#include <QWidget>
#include <QVBoxLayout>

#include "panelbutton.h"
#include "panelpluginterface.h"
#include "deskbutton.h"

class deskswitch : public panelbutton, panelpluginterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.deskswitch.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    deskswitch();
    ~deskswitch();

    //begin plugininterface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* event);
    QHash<QString, QString> getpluginfo();
    //end plugininterface

signals:
    void activate(int desk);

private slots:
    void setupbts();
    void switchtodesk(int num);

private:
    QHBoxLayout *basehlayout;
    QList<deskbutton*> dbuttons;
    popupmenu *pmenu;
    int deskcount = 0;
    xcb_atom_t _net_client_list;
    xcb_atom_t _net_wm_desktop;
    xcb_atom_t _net_current_desktop;
};

#endif // DESKSWITCH_H
