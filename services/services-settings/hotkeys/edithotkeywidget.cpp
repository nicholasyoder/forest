// SPDX-License-Identifier: LGPL-3.0-or-later

#include "edithotkeywidget.h"
#include "ui_edithotkeywidget.h"

edithotkeywidget::edithotkeywidget(QWidget *parent) : QWidget(parent), ui(new Ui::edithotkeywidget){
    ui->setupUi(this);
    ui->commandRbt->setChecked(true);
}

edithotkeywidget::~edithotkeywidget(){
    delete ui;
}

void edithotkeywidget::set_data(const HotkeyData& data){
    ui->shortcutbt->setText(data.shortcut);
    ui->descriptionTbox->setText(data.description);

    if (auto commandAction = data.action.dynamicCast<CommandAction>()) {
        ui->commandRbt->setChecked(true);
        ui->commandTbox->setText(commandAction->command);
    }
    else if (auto dbusAction = data.action.dynamicCast<CustomDBusAction>()) {
        ui->customdbusRbt->setChecked(true);
        ui->serviceTbox->setText(dbusAction->service);
        ui->pathTbox->setText(dbusAction->path);
        ui->interfaceTbox->setText(dbusAction->interface);
        ui->methodTbox->setText(dbusAction->method);
        ui->busCbox->setCurrentIndex(dbusAction->isSystemBus ? 1 : 0);
    }
}

void edithotkeywidget::set_hotkeys_paused(bool pause){
    if (pause == hotkeys_paused) return;
    hotkeys_paused = pause;
    miscutills::call_dbus(pause ? "forest/hotkeys/pauseHotkeys"
                                : "forest/hotkeys/resumeHotkeys");
}

void edithotkeywidget::keyPressEvent(QKeyEvent *event){
    if (waitingforkeys == true){
        if (event->key()==Qt::Key_AltGr||event->key()==Qt::Key_Print||event->key()==Qt::Key_CapsLock||event->key()==Qt::Key_NumLock||
                event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) { return; }
        else if (event->key()==Qt::Key_Control){ keys = keys + "Ctrl+"; return; }
        else if (event->key()==Qt::Key_Shift){ keys = keys + "Shift+"; return; }
        else if (event->key()==Qt::Key_Alt) { keys = keys + "Alt+"; return; }
        else if (event->key()==Qt::Key_Meta || event->key() == 16777299){ keys = keys + "Meta+"; return; }

        QKeySequence key = event->key();
        keys = keys + key.toString();

        waitingforkeys = false;
        set_hotkeys_paused(false);
        ui->shortcutbt->setText(keys);
        keys = "";
    }
}

void edithotkeywidget::closeEvent(QCloseEvent *event){
    set_hotkeys_paused(false);
    QWidget::closeEvent(event);
}

void edithotkeywidget::on_okbt_clicked(){
    HotkeyData data;
    data.shortcut = ui->shortcutbt->text();
    data.description = ui->descriptionTbox->text();

    if (ui->commandRbt->isChecked()){
        data.action = QSharedPointer<HotkeyAction>(new CommandAction(ui->commandTbox->text()));
    }
    else{
        data.action = QSharedPointer<HotkeyAction>(new CustomDBusAction(
            ui->serviceTbox->text(),
            ui->pathTbox->text(),
            ui->interfaceTbox->text(),
            ui->methodTbox->text(),
            ui->busCbox->currentText() == "system"
        ));
    }

    emit data_updated(data);
    this->close();
}

void edithotkeywidget::on_cancelbt_clicked(){
    this->close();
}

void edithotkeywidget::on_commandRbt_toggled(bool checked){
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(0);
}

void edithotkeywidget::on_customdbusRbt_toggled(bool checked){
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(1);
}

void edithotkeywidget::on_builtindbusRbt_toggled(bool checked){
    if (checked == true)
        ui->stackedWidget->setCurrentIndex(2);
}

void edithotkeywidget::on_shortcutbt_clicked(){
    ui->shortcutbt->setText("Press Keys");
    keys.clear();
    waitingforkeys = true;
    set_hotkeys_paused(true);
}
