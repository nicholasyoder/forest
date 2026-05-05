// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITHOTKEYWIDGET_H
#define EDITHOTKEYWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QSharedPointer>

class HotkeyAction : public QObject{
    Q_OBJECT
public:
    virtual ~HotkeyAction() = default;
    virtual QString type() const = 0;
};

class CommandAction : public HotkeyAction{
    Q_OBJECT
public:
    CommandAction(const QString& cmd) : command(cmd) {}
    QString type() const override { return "exec"; }
    QString command;
};

class CustomDBusAction : public HotkeyAction{
    Q_OBJECT
public:
    CustomDBusAction(const QString& svc, const QString& p, const QString& iface, const QString& m, bool isSystem)
        : service(svc), path(p), interface(iface), method(m), isSystemBus(isSystem) {}
    QString type() const override { return "dbus"; }
    QString service;
    QString path;
    QString interface;
    QString method;
    bool isSystemBus;
};

class HotkeyData{
public:
    QString shortcut;
    QString description;
    QSharedPointer<HotkeyAction> action;
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
private slots:
    void on_okbt_clicked();
    void on_cancelbt_clicked();
    void on_commandRbt_toggled(bool checked);
    void on_customdbusRbt_toggled(bool checked);
    void on_builtindbusRbt_toggled(bool checked);
    void on_shortcutbt_clicked();
private:
    Ui::edithotkeywidget *ui;
    bool waitingforkeys = false;
    QString keys;
};

#endif // EDITHOTKEYWIDGET_H
