// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FVOLUME_H
#define FVOLUME_H

#include <QWidget>
#include <QHBoxLayout>
#include <QSlider>
#include <QIcon>
#include <QGenericPlugin>
#include <QtDBus>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"
#include "popupmenu.h"

#include "audioengine.h"
#include "audiodevice.h"
#include "alsaengine.h"

class fvolume : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.volume.plugin")
    Q_INTERFACES(panelpluginterface)

public:
    fvolume();

    //begin plugin interface
    void setupPlug(QBoxLayout *layout, QList<pmenuitem*> itemlist);
    void closePlug(){this->close(); deleteLater();}
    void XcbEventFilter(xcb_generic_event_t* /*event*/){}
    QHash<QString, QString> getpluginfo();
    //end plugin interface

public slots:
    Q_SCRIPTABLE void volumeup(){setvolume(master_volume+2);}
    Q_SCRIPTABLE void volumedown(){setvolume(master_volume-2);}
    Q_SCRIPTABLE void togglemuted();

protected:
    void wheelEvent(QWheelEvent *event);

private slots:
    void loadsettings();
    void showsettings();
    void save_volumes();
    void setvolume(int value);
    //void togglemuted();
    void volumechanged(int value);
    void mutechanged(bool state);
    void updateicon();
    void handlemouseReleased(QMouseEvent *event);

private:
    int master_volume = 0;
    bool master_muted = false;
    bool autosave = false;

    AudioEngine *audioengine = nullptr;
    AudioDevice *master_device = nullptr;

    popup *popupbox;
    QVBoxLayout * popup_layout = nullptr;
    popupmenu *pmenu;

    RunOnce* save_runner = nullptr;
};
#endif // FVOLUME_H
