/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
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

// This code is mostly copied from LxQt
// Where it was preceded by the following comment:

// the following XKB code is taken from numlockx which is licensed under MIT.
// Copyright (C) 2000-2001 Lubos Lunak        <l.lunak@kde.org>
// Copyright (C) 2001      Oswald Buddenhagen <ossi@kde.org>

#include "numlock.h"
#include <string.h>
#include <stdlib.h>
#include <QtX11Extras/QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

/* the XKB stuff is based on code created by Oswald Buddenhagen <ossi@kde.org> */

static unsigned int xkb_mask_modifier(Display* /*dpy*/, XkbDescPtr xkb, const char *name ){
    int i;
    if( !xkb || !xkb->names )
        return 0;
    for( i = 0; i < XkbNumVirtualMods; i++ ) {
        char* modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
        if( modStr != nullptr && strcmp(name, modStr) == 0 ) {
            unsigned int mask;
            XkbVirtualModsToReal( xkb, 1 << i, &mask );
            return mask;
        }
    }
    return 0;
}

static unsigned int xkb_numlock_mask(Display* dpy){
    XkbDescPtr xkb;
    if(( xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd )) != nullptr ) {
        unsigned int mask = xkb_mask_modifier( dpy, xkb, "NumLock" );
        XkbFreeKeyboard( xkb, 0, True );
        return mask;
    }
    return 0;
}

static int xkb_set_on(Display* dpy){
    unsigned int mask;
    mask = xkb_numlock_mask(dpy);
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, mask);
    return 1;
}

void numlock::enableNumlock(){
    // this currently only works for X11
    if(QX11Info::isPlatformX11())
        xkb_set_on(QX11Info::display());
}
