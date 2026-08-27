// SPDX-License-Identifier: LGPL-3.0-or-later

#include "services.h"

services::services(){

}

void services::setupPlug(){
    fhotkeys = new foresthotkeys;
    fhotkeys->setup();

    fnotify = new notify;
    fnotify->setup();

    fpolkit = new polkitagent;
}
