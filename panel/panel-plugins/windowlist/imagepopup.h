#ifndef IMAGEPOPUP_H
#define IMAGEPOPUP_H

#include <QLabel>
#include <QGridLayout>
#include <QTimer>

#include "popup.h"
#include "windowbutton.h"
#include "closebutton.h"

#include "xcbutills/xcbutills.h"

class imagepopup : public QObject
{
    Q_OBJECT
public:
    imagepopup(QWidget *parentw);

signals:

public slots:
    void btmouseEnter(windowbutton *bt);
    void btmouseLeave();
    void btclicked();
    void set_enabled(bool enabled = true){ popup_enabled = enabled; }

private slots:
    void showpopup();
    void tryclosepopup();
    void closepopup();
    void deleteopenptimer();
    void closewindow(){if(currentbt){ Xcbutills::closeWindow(currentbt->windowId()); closepopup();}}
    void raisewindow(){if(currentbt){ Xcbutills::raiseWindow(currentbt->windowId()); closepopup();}}
    //void resizepbox(){pbox->resize(pbox->sizeHint()); pbox->positionOnLauncher();}

private:
    QPixmap get_window_image();

    bool popup_enabled = true;

    windowbutton *currentbt = nullptr;
    bool open = false;

    QTimer *openptimer = nullptr;
    QTimer *closeptimer = nullptr;
    popup *pbox = nullptr;
    QLabel *scrshotlabel = nullptr;
    QLabel *wintitlelabel = nullptr;
    //QVBoxLayout *popupvlayout = nullptr;
    QGridLayout *popupglayout = nullptr;
    QWidget *parentwidget = nullptr;
};

#endif // IMAGEPOPUP_H
