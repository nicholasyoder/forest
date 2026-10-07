// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITHOTKEYWIDGET_H
#define EDITHOTKEYWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDebug>
#include "miscutills.h"

class HotkeyData{
public:
    QString shortcut;
    QString description;
    QString action; // Forest.conf form: a command or a DBUS: action
};

namespace Ui { class edithotkeywidget; }
class edithotkeywidget : public QWidget{
    Q_OBJECT
public:
    explicit edithotkeywidget(QWidget *parent = nullptr);
    ~edithotkeywidget();
    void set_data(const HotkeyData& data);
signals:
    void data_updated(const HotkeyData& data);
protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    void on_okbt_clicked();
    void on_cancelbt_clicked();
    void on_commandRbt_toggled(bool checked);
    void on_customdbusRbt_toggled(bool checked);
    void on_builtindbusRbt_toggled(bool checked);
    void on_shortcutbt_clicked();
    void on_builtindbusLwidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    Ui::edithotkeywidget *ui;
    bool waitingforkeys = false;
    bool hotkeys_paused = false;
    QString keys;
    void set_hotkeys_paused(bool pause);
    void finish_capture();
    void load_builtins();
    QListWidgetItem *find_builtin(const QString &action) const;
};

#endif // EDITHOTKEYWIDGET_H
