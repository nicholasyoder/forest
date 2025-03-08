#ifndef POWERMAN_H
#define POWERMAN_H

#include <QWidget>
#include <QTimer>
#include <QtDBus>
#include <QApplication>
#include <QKeyEvent>

#include "iconbutton.h"
#include "imagewidget.h"

enum class ActionType {SHUTDOWN, REBOOT, LOGOUT, SUSPEND, HIBERNATE};
enum class FadeDirection {FADEIN, FADEOUT};

class logoutmanager : public QWidget{
    Q_OBJECT
public:
    logoutmanager();
    ~logoutmanager();

public slots:
    void startbackfade();
    void activate(){this->activateWindow();}

private slots:
    void keyPressEvent(QKeyEvent *event);
    void do_fade(FadeDirection direction, int interval = 20, float limit = 0);
    void perform_fade_step(FadeDirection direction, float limit);
    void set_initial_focus();
    void start_action(ActionType action);
    void do_action(ActionType action);
    void cancel();

private:
    void setup();
    QList<imagewidget*> background_faders;
    iconbutton *focusbt = nullptr;
    QSettings *settings = nullptr;
    QTimer* fade_timer = nullptr;
    float fade_opacity = 0;
};

#endif // POWERMAN_H
