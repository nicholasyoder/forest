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

#include "vpopupwidget.h"

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
    Q_SCRIPTABLE void volumeup(){setvolume(volume+2);}
    Q_SCRIPTABLE void volumedown(){setvolume(volume-2);}
    Q_SCRIPTABLE void togglemuted();

protected:
    void wheelEvent(QWheelEvent *event);

private slots:
    void setvolume(int value);
    //void togglemuted();
    void volumechanged(int value);
    void mutechanged(bool state);
    void updateicon();
    void handlemouseReleased(QMouseEvent *event);

private:
    int volume = 0;
    bool muted = false;

    AudioEngine *audioengine;
    AudioDevice *audiodevice;

    popup *popupbox;
    popupmenu *pmenu;
    QSlider *slider;
};
#endif // FVOLUME_H
