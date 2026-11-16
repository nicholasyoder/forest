// SPDX-License-Identifier: LGPL-3.0-or-later

#include "edithotkeywidget.h"
#include "ui_edithotkeywidget.h"

#include "displayprofiles.h"
#include "hotkeyconfig.h"

#include <QApplication>
#include <QMessageBox>

namespace {
constexpr int kActionRole = Qt::UserRole;
constexpr int kDescriptionRole = Qt::UserRole + 1;
}

edithotkeywidget::edithotkeywidget(QWidget *parent) : QDialog(parent), ui(new Ui::edithotkeywidget){
    ui->setupUi(this);
    setModal(true);
    ui->commandRbt->setChecked(true);
    connect(ui->repeatCbox, &QCheckBox::toggled, ui->repeatIntervalSbox, &QSpinBox::setEnabled);
    connect(ui->okbt, &QPushButton::clicked, this, &edithotkeywidget::accept_data);
    connect(ui->cancelbt, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->shortcutbt, &QPushButton::clicked, this, &edithotkeywidget::start_capture);
    connect(ui->commandRbt, &QRadioButton::toggled, this, [this](bool on){ if (on) ui->stackedWidget->setCurrentIndex(0); });
    connect(ui->customdbusRbt, &QRadioButton::toggled, this, [this](bool on){ if (on) ui->stackedWidget->setCurrentIndex(1); });
    connect(ui->builtindbusRbt, &QRadioButton::toggled, this, [this](bool on){ if (on) ui->stackedWidget->setCurrentIndex(2); });
    connect(ui->builtindbusLwidget, &QListWidget::currentItemChanged, this, &edithotkeywidget::builtin_item_changed);
}

// Also runs from the destructor path via done(): hotkeys never stay paused.
edithotkeywidget::~edithotkeywidget(){
    end_capture();
    delete ui;
}

void edithotkeywidget::done(int result){
    end_capture();
    QDialog::done(result);
}

void edithotkeywidget::set_data(const HotkeyData& data){
    trigger = data.trigger;
    show_trigger();
    ui->descriptionTbox->setText(data.description);
    ui->repeatCbox->setChecked(data.repeat);
    ui->repeatIntervalSbox->setEnabled(data.repeat);
    ui->repeatIntervalSbox->setValue(data.repeatInterval);
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
        const auto candidate = hotkeyconfig::parseDBusAction(item->data(kActionRole).toString());
        if (candidate && candidate->sameTarget(*parsed))
            return item;
    }
    return nullptr;
}

// Follows the selection unless the user wrote their own description.
void edithotkeywidget::builtin_item_changed(QListWidgetItem *current, QListWidgetItem *previous){
    const QString description = ui->descriptionTbox->text();
    if (current && (description.isEmpty() || (previous && description == previous->data(kDescriptionRole).toString())))
        ui->descriptionTbox->setText(current->data(kDescriptionRole).toString());
}

static bool is_meta_key(int key){
    return key == Qt::Key_Meta || key == Qt::Key_Super_L || key == Qt::Key_Super_R;
}

static bool is_modifier_key(int key){
    return is_meta_key(key) || key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt
        || key == Qt::Key_AltGr || key == Qt::Key_CapsLock || key == Qt::Key_NumLock
        || key == Qt::Key_ScrollLock || key == Qt::Key_Hyper_L || key == Qt::Key_Hyper_R;
}

void edithotkeywidget::show_trigger(){
    ui->shortcutbt->setText(trigger.isEmpty() ? QStringLiteral("Click to set")
                                              : hotkeyconfig::triggerDisplay(trigger));
}

void edithotkeywidget::start_capture(){
    if (waitingforkeys) return;
    ui->shortcutbt->setText("Press keys (Esc cancels)");
    meta_pending.clear();
    waitingforkeys = true;
    qApp->installEventFilter(this);
    set_hotkeys_paused(true);
}

void edithotkeywidget::end_capture(){
    if (waitingforkeys){
        waitingforkeys = false;
        qApp->removeEventFilter(this);
        show_trigger();
    }
    set_hotkeys_paused(false);
}

bool edithotkeywidget::eventFilter(QObject *watched, QEvent *event){
    if (waitingforkeys && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)){
        // Other windows' keys (none while modal) aren't ours to eat.
        if (auto *widget = qobject_cast<QWidget *>(watched); widget && widget->window() != this) return false;
        auto *key_event = static_cast<QKeyEvent *>(event);
        if (event->type() == QEvent::KeyPress) handle_capture_press(key_event);
        else handle_capture_release(key_event);
        return true;
    }
    return QDialog::eventFilter(watched, event);
}

// Records the key's xkb keysym (nativeVirtualKey) so F-keys, media keys etc. work.
void edithotkeywidget::handle_capture_press(QKeyEvent *event){
    if (event->isAutoRepeat()) return;
    const int key = event->key();
    if (is_meta_key(key)){
        meta_pending = QStringLiteral("LOGO");
        return;
    }
    if (is_modifier_key(key)){
        meta_pending.clear(); // Meta+Shift etc. is not a bare tap
        return;
    }

    const Qt::KeyboardModifiers mods = event->keyCombination().keyboardModifiers();
    if (key == Qt::Key_Escape && mods == Qt::NoModifier){
        end_capture(); // restores the previous shortcut
        return;
    }

    const QString candidate = hotkeyconfig::triggerFromKeysym(mods, event->nativeVirtualKey());
    if (candidate.isEmpty()){
        ui->shortcutbt->setText("Unsupported key, try again");
        return;
    }
    if (const QString refusal = hotkeyconfig::triggerRefusal(candidate); !refusal.isEmpty()){
        ui->shortcutbt->setText("Needs Ctrl, Alt or Meta, try again");
        ui->shortcutbt->setToolTip(refusal);
        return;
    }
    ui->shortcutbt->setToolTip(QString());
    trigger = candidate;
    end_capture();
}

// A lone Meta tap is a bare "LOGO" trigger.
void edithotkeywidget::handle_capture_release(QKeyEvent *event){
    if (!event->isAutoRepeat() && is_meta_key(event->key()) && meta_pending == "LOGO"){
        trigger = meta_pending;
        end_capture();
    }
}

void edithotkeywidget::set_hotkeys_paused(bool pause){
    if (pause == hotkeys_paused) return;
    hotkeys_paused = pause;
    miscutills::call_dbus(pause ? "forest/hotkeys/pauseHotkeys"
                                : "forest/hotkeys/resumeHotkeys");
}

void edithotkeywidget::accept_data(){
    if (trigger.isEmpty()){
        QMessageBox::information(this, "Hotkey", "Click the shortcut button and press the keys to use.");
        return;
    }
    HotkeyData data;
    data.trigger = trigger;
    data.description = ui->descriptionTbox->text();
    data.repeat = ui->repeatCbox->isChecked();
    data.repeatInterval = ui->repeatIntervalSbox->value();

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
    accept();
}
