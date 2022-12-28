#ifndef WINDOWBUTTON_H
#define WINDOWBUTTON_H

#include "panelbutton.h"

#include "popupmenu.h"

class windowbutton : public panelbutton
{
    Q_OBJECT

public:
    explicit windowbutton(ulong window, int desktop, QIcon icon, QString text);

    ulong windowId(){return window_id;}
    void setWindowDesktop(int desktop){window_desktop = desktop;}
    int windowDesktop(){return window_desktop;}

signals:

private slots:
    void raise_w();
    void maximize_w();
    void minimize_w();
    void close_w();
    void demaximize_w();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private:
    ulong window_id;
    int window_desktop;

    popupmenu *pmenu = nullptr;
};

#endif // WINDOWBUTTON_H
