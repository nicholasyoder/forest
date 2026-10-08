// SPDX-License-Identifier: LGPL-3.0-or-later

#include "edithotkeywidget.h"
#include "ui_edithotkeywidget.h"

#include "displayprofiles.h"
#include "hotkeyconfig.h"

namespace {
constexpr int kActionRole = Qt::UserRole;
constexpr int kDescriptionRole = Qt::UserRole + 1;
}

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
    load_builtins();

    const auto dbusAction = hotkeyconfig::parseDBusAction(data.action);
    if (!dbusAction) {
        ui->commandRbt->setChecked(true);
        ui->commandTbox->setText(data.action);
    }
    else if (QListWidgetItem *builtin = find_builtin(data.action)) {
        ui->builtindbusRbt->setChecked(true);
        const QSignalBlocker blocker(ui->builtindbusLwidget); // keep the description as saved
        ui->builtindbusLwidget->setCurrentItem(builtin);
    }
    else {
        ui->customdbusRbt->setChecked(true);
        ui->serviceTbox->setText(dbusAction->service);
        ui->pathTbox->setText(dbusAction->path);
        ui->interfaceTbox->setText(dbusAction->interface);
        ui->methodTbox->setText(dbusAction->method);
        ui->argTbox->setText(dbusAction->arg);
        ui->busCbox->setCurrentIndex(dbusAction->systemBus ? 1 : 0);
    }
}

void edithotkeywidget::load_builtins(){
    const QSignalBlocker blocker(ui->builtindbusLwidget);
    ui->builtindbusLwidget->clear();
    auto add = [this](const QString &description, const DBusHotkeyAction &action, const QString &note = QString()){
        auto *item = new QListWidgetItem(description, ui->builtindbusLwidget);
        item->setData(kActionRole, hotkeyconfig::formatDBusAction(action));
        item->setData(kDescriptionRole, description);
        item->setToolTip(note);
    };

    for (const BuiltinHotkeyAction &builtin : hotkeyconfig::builtinActions())
        add(builtin.description, builtin.action, builtin.note);
    add("Next display profile", hotkeyconfig::nextDisplayProfileAction());
    DisplayProfiles profiles;
    profiles.load();
    for (const DisplayProfile &profile : DisplayProfiles::sortedByName(profiles.profiles()))
        add(hotkeyconfig::displayProfileDescription(profile.name), hotkeyconfig::displayProfileAction(profile.id));
}

// Compares parsed fields: older entries were written in QHash key order.
QListWidgetItem *edithotkeywidget::find_builtin(const QString &action) const{
    const auto parsed = hotkeyconfig::parseDBusAction(action);
    if (!parsed) return nullptr;
    for (int i = 0; i < ui->builtindbusLwidget->count(); i++){
        QListWidgetItem *item = ui->builtindbusLwidget->item(i);
        if (hotkeyconfig::parseDBusAction(item->data(kActionRole).toString())->sameTarget(*parsed))
            return item;
    }
    return nullptr;
}

// Follows the selection unless the user wrote their own description.
void edithotkeywidget::on_builtindbusLwidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    const QString description = ui->descriptionTbox->text();
    if (current && (description.isEmpty() || (previous && description == previous->data(kDescriptionRole).toString())))
        ui->descriptionTbox->setText(current->data(kDescriptionRole).toString());
}

void edithotkeywidget::set_hotkeys_paused(bool pause){
    if (pause == hotkeys_paused) return;
    hotkeys_paused = pause;
    miscutills::call_dbus(pause ? "forest/hotkeys/pauseHotkeys"
                                : "forest/hotkeys/resumeHotkeys");
}

static bool is_meta_key(int key){
    return key == Qt::Key_Meta || key == Qt::Key_Super_L || key == Qt::Key_Super_R;
}

void edithotkeywidget::keyPressEvent(QKeyEvent *event){
    if (waitingforkeys == true){
        if (event->isAutoRepeat()) return;
        if (event->key()==Qt::Key_AltGr||event->key()==Qt::Key_Print||event->key()==Qt::Key_CapsLock||event->key()==Qt::Key_NumLock||
                event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) { return; }
        else if (event->key()==Qt::Key_Control){ keys = keys + "Ctrl+"; return; }
        else if (event->key()==Qt::Key_Shift){ keys = keys + "Shift+"; return; }
        else if (event->key()==Qt::Key_Alt) { keys = keys + "Alt+"; return; }
        else if (is_meta_key(event->key())){ keys = keys + "Meta+"; return; }

        QKeySequence key = event->key();
        keys = keys + key.toString();
        finish_capture();
    }
}

// A lone Meta tap: "Meta" is the value foresthotkeys maps to a bare LOGO trigger.
void edithotkeywidget::keyReleaseEvent(QKeyEvent *event){
    if (waitingforkeys && !event->isAutoRepeat() && is_meta_key(event->key()) && keys == "Meta+"){
        keys = "Meta";
        finish_capture();
    }
}

void edithotkeywidget::finish_capture(){
    waitingforkeys = false;
    set_hotkeys_paused(false);
    ui->shortcutbt->setText(keys);
    keys = "";
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
        data.action = ui->commandTbox->text();
    }
    else if (ui->customdbusRbt->isChecked()){
        DBusHotkeyAction action;
        action.service = ui->serviceTbox->text();
        action.path = ui->pathTbox->text();
        action.interface = ui->interfaceTbox->text();
        action.method = ui->methodTbox->text();
        action.arg = ui->argTbox->text();
        action.systemBus = ui->busCbox->currentIndex() == 1;
        data.action = hotkeyconfig::formatDBusAction(action);
    }
    else if (QListWidgetItem *builtin = ui->builtindbusLwidget->currentItem()){
        data.action = builtin->data(kActionRole).toString();
    }
    else {
        return; // built-in type with nothing picked
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
