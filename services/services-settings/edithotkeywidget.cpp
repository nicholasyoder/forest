/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021-2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

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

void edithotkeywidget::keyPressEvent(QKeyEvent *event){
    if (waitingforkeys == true){
        // Need to ungrab the global hotkeys while listening for this
        // Also need to check if the new sequence conflicts with existing hotkey
        if (event->key()==Qt::Key_AltGr||event->key()==Qt::Key_Print||event->key()==Qt::Key_CapsLock||event->key()==Qt::Key_NumLock||
                event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) { return; }
        else if (event->key()==Qt::Key_Control){ keys = keys + "Ctrl+"; return; }
        else if (event->key()==Qt::Key_Shift){ keys = keys + "Shift+"; return; }
        else if (event->key()==Qt::Key_Alt) { keys = keys + "Alt+"; return; }
        else if (event->key()==Qt::Key_Meta || event->key() == 16777299){ keys = keys + "Meta+"; return; }

        QKeySequence key = event->key();
        keys = keys + key.toString();

        waitingforkeys = false;
        ui->shortcutbt->setText(keys);
        keys = "";
    }
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
}
