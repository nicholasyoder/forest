#ifndef FORESTXCBEVENTFILTER_H
#define FORESTXCBEVENTFILTER_H

#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>
#include "forest.h"

class forestXcbEventFilter : public QAbstractNativeEventFilter{
public:
    forest *f = nullptr;

    //Get Xcb events (X client list change, Active window change, etc.) and send them into forest::XcbEventFilter
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override{
        if (eventType == "xcb_generic_event_t"){
            xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
            f->XcbEventFilter(ev);
        }
        return false;
    }
};

#endif // FORESTXCBEVENTFILTER_H
