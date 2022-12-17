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

#include "vpopupwidget.h"

vpopupwidget::vpopupwidget()
{
    Qt::WindowFlags flags;
    flags |= Qt::X11BypassWindowManagerHint;
    flags |= Qt::Popup;
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::CustomizeWindowHint;
    this->setWindowFlags(flags);

    QHBoxLayout *hlayout = new QHBoxLayout;

    hlayout->addWidget(vslider);
    connect(vslider, SIGNAL(valueChanged(int)), this, SLOT(setvolume()));
    this->setLayout(hlayout);

    vslider->setRange(0,100);
    //currentsink = getsink();

    this->setFixedHeight(150);
}

/*void vpopupwidget::setvolume()
{
    QProcess *p = new QProcess;
    QString svol;
    svol.setNum(vslider->value());
    p->start("pactl set-sink-volume " + currentsink + " " + svol + "%");
}

QString vpopupwidget::getsink()
{
    return "alsa_output.pci-0000_00_1e.2.analog-stereo";
}

void vpopupwidget::exec(int x, int y)
{
    this->move(x,y);
    this->show();
}
*/
