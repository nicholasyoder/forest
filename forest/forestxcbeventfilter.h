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

#ifndef FORESTXCBEVENTFILTER_H
#define FORESTXCBEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QDebug>

#include <xcb/xcb.h>
#include "forest.h"

class forestXcbEventFilter : public QAbstractNativeEventFilter
{

public:
    forest *f = nullptr;

    //Get Xcb events (X client list change, Active window change, etc.) and send them into forest::XcbEventFilter
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override
    {
        //qDebug() << eventType;
        if (eventType == "xcb_generic_event_t")
        {
            xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
            f->XcbEventFilter(ev);
        }
        return false;
    }
};

#endif // FORESTXCBEVENTFILTER_H
