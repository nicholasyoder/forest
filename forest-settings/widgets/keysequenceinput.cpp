#include "keysequenceinput.h"

keysequenceinput::keysequenceinput(){
    setCheckable(true);
    connect(this, &keysequenceinput::toggled, this, &keysequenceinput::bttoggled);
}

void keysequenceinput::setsequence(QString seq){
    keys = seq;
    setText(keys);
}

void keysequenceinput::keyPressEvent(QKeyEvent *event){
    if (waitingforkeys == true)
    {
        if (event->key()==Qt::Key_AltGr||event->key()==Qt::Key_Print||event->key()==Qt::Key_CapsLock||event->key()==Qt::Key_NumLock||
                event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) { return; }
        else if (event->key()==Qt::Key_Control){ keys = keys + "Ctrl+"; return; }
        else if (event->key()==Qt::Key_Shift){ keys = keys + "Shift+"; return; }
        else if (event->key()==Qt::Key_Alt) { keys = keys + "Alt+"; return; }
        else if (event->key()==Qt::Key_Meta || event->key() == 16777299){ keys = keys + "Meta+"; return; }

        QKeySequence key = event->key();
        keys = keys + key.toString();

        waitingforkeys = false;
        setText(keys);
        setChecked(false);
        emit sequenceChanged(keys);
    }
}

void keysequenceinput::bttoggled(bool checked){
    if (checked){
        setText("Press Keys");
        oldkeys = keys;
        keys.clear();
        waitingforkeys = true;
    }
    else {
        if (keys == ""){
            keys = oldkeys;
            setText(keys);
            waitingforkeys = false;
        }
    }
}
