#ifndef IMAGEPOPUP_H
#define IMAGEPOPUP_H

#include <QLabel>
#include <QGridLayout>

#include "popup.h"
#include "button.h"
#include "closebutton.h"

#include "xcbutills/xcbutills.h"

class imagepopup : public QObject
{
    Q_OBJECT
public:
    imagepopup(QWidget *parentw);

signals:

public slots:
    void btmouseEnter(button *bt);
    void btmouseLeave();
    void btclicked();

private slots:
    void showpopup();
    void tryclosepopup();
    void closepopup();
    void deleteopenptimer();
    void closewindow(){if(currentbt){ Xcbutills::closeWindow(currentbt->xcbwindow()); closepopup();}}
    void raisewindow(){if(currentbt){ Xcbutills::raiseWindow(currentbt->xcbwindow()); closepopup();}}
    //void resizepbox(){pbox->resize(pbox->sizeHint()); pbox->positionOnLauncher();}

private:
    button *currentbt = nullptr;
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
