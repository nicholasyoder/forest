// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITHOTKEYWIDGET_H
#define EDITHOTKEYWIDGET_H

#include <QDialog>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QDebug>
#include "miscutills.h"

class HotkeyData{
public:
    QString trigger; // spec-grammar, see hotkeyconfig::kTriggerKey
    QString description;
    QString action; // Forest.conf form: a command or a DBUS: action
    bool repeat = false;
    int repeatInterval = 0; // ms
};

namespace Ui { class edithotkeywidget; }
class edithotkeywidget : public QDialog{
    Q_OBJECT
public:
    explicit edithotkeywidget(QWidget *parent = nullptr);
    ~edithotkeywidget();
    void set_data(const HotkeyData& data);
    void done(int result) override;
signals:
    void data_updated(const HotkeyData& data);
protected:
    // While capturing, takes every key press/release in the app.
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void accept_data();
    void start_capture();
    void builtin_item_changed(QListWidgetItem *current, QListWidgetItem *previous);
private:
    Ui::edithotkeywidget *ui;
    bool waitingforkeys = false;
    bool hotkeys_paused = false;
    QString trigger; // the shown shortcut
    QString meta_pending; // set while only Meta is held
    void set_hotkeys_paused(bool pause);
    void handle_capture_press(QKeyEvent *event);
    void handle_capture_release(QKeyEvent *event);
    void end_capture();
    void show_trigger();
    void load_builtins();
    QListWidgetItem *find_builtin(const QString &action) const;
};

#endif // EDITHOTKEYWIDGET_H
