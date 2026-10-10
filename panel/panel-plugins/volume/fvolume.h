// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FVOLUME_H
#define FVOLUME_H

#include <QWidget>
#include <QMenu>
#include <QHBoxLayout>
#include <QSlider>
#include <QIcon>
#include <QGenericPlugin>
#include <QtDBus>

#include "panelpluginterface.h"
#include "panelbutton.h"
#include "popup.h"

#include "audioengine.h"
#include "audiodevice.h"
#include "alsaengine.h"

class fvolume : public panelbutton, panelpluginterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "forest.panel.volume.plugin" FILE "volume.json")
    Q_INTERFACES(panelpluginterface)

public:
    fvolume();
    ~fvolume() override { delete pmenu; }

    //begin plugin interface
    void setupPlug(QBoxLayout *layout, QList<QAction*> itemlist);
    void closePlug(){this->close(); deleteLater();}
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
    QMenu *pmenu = nullptr;

    RunOnce* save_runner = nullptr;
};
#endif // FVOLUME_H
