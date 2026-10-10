// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingsbinder.h"
#include "settingsrow.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaProperty>

namespace {

const int typed_delay = 500;
const int saved_ms = 1000;
const int error_ms = 3000;

bool saves_immediately(QWidget *widget){
    if (qobject_cast<QAbstractButton*>(widget)) return true;
    QComboBox *combo = qobject_cast<QComboBox*>(widget);
    return combo && !combo->isEditable();
}

}

SettingsBinder::SettingsBinder(const QString &organization, const QString &application,
                               const QString &group, QObject *parent)
    : QObject(parent), organization(organization), application(application), group(group)
{
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &SettingsBinder::flush);
    // Closing the window mid-debounce would drop the edit.
    connect(qApp, &QCoreApplication::aboutToQuit, this, &SettingsBinder::flush);
}

void SettingsBinder::bind(QWidget *widget, const QString &key, const QVariant &default_value,
                          const Converter &converter){
    QMetaProperty prop = widget->metaObject()->userProperty();
    if (!prop.isValid() || !prop.hasNotifySignal()) {
        qWarning() << "SettingsBinder: no notifying USER property on" << widget;
        return;
    }
    bindings[widget] = {key, default_value, converter};
    QMetaMethod slot = metaObject()->method(metaObject()->indexOfSlot("control_changed()"));
    connect(widget, prop.notifySignal(), this, slot);
    connect(widget, &QObject::destroyed, this, [this, widget]{ bindings.remove(widget); });
}

void SettingsBinder::changed(QWidget *row, std::function<void(QSettings&)> write){
    if (loading) return;
    writers.append({row, write});
    schedule(true);
}

SettingsBinder::Converter SettingsBinder::item_data(QComboBox *combo){
    QPointer<QComboBox> c(combo);
    return {
        [c](const QVariant &stored){ return c ? QVariant(c->itemText(c->findData(stored))) : QVariant(); },
        [c](const QVariant &text){ return c ? c->itemData(c->findText(text.toString())) : QVariant(); },
    };
}

SettingsBinder::Converter SettingsBinder::percent(){
    return {
        [](const QVariant &stored){ return QVariant(qRound(stored.toDouble() * 100)); },
        [](const QVariant &shown){ return QVariant(shown.toDouble() / 100); },
    };
}

QSettings *SettingsBinder::open_settings(){
    QSettings *settings = new QSettings(organization, application);
    if (!group.isEmpty()) settings->beginGroup(group);
    return settings;
}

void SettingsBinder::load(){
    flush();
    std::unique_ptr<QSettings> settings(open_settings());
    loading = true;
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
        QMetaProperty prop = it.key()->metaObject()->userProperty();
        QVariant value = settings->value(it->key, it->default_value);
        if (it->converter.to_widget) value = it->converter.to_widget(value);
        // QSettings returns strings for values read from file.
        value.convert(prop.metaType());
        prop.write(it.key(), value);
    }
    loading = false;
}

void SettingsBinder::control_changed(){
    QWidget *widget = qobject_cast<QWidget*>(sender());
    if (loading || !widget || !bindings.contains(widget)) return;
    if (!dirty.contains(widget)) dirty.append(widget);
    schedule(saves_immediately(widget));
}

void SettingsBinder::schedule(bool immediate){
    if (immediate)
        timer.start(0);
    else if (!(timer.isActive() && timer.interval() == 0))
        timer.start(typed_delay);
}

void SettingsBinder::flush(){
    timer.stop();
    if (dirty.isEmpty() && writers.isEmpty()) return;

    std::unique_ptr<QSettings> settings(open_settings());
    QList<QPointer<QWidget>> rows;
    foreach (const QPointer<QWidget> &widget, dirty) {
        if (!widget || !bindings.contains(widget)) continue;
        const Binding &binding = bindings[widget];
        QVariant value = widget->metaObject()->userProperty().read(widget);
        if (binding.converter.to_stored) value = binding.converter.to_stored(value);
        settings->setValue(binding.key, value);
        rows.append(widget);
    }
    for (const auto &writer : writers) {
        writer.second(*settings);
        rows.append(writer.first);
    }
    dirty.clear();
    writers.clear();

    settings->sync();
    bool ok = settings->status() == QSettings::NoError;
    if (!ok)
        qWarning() << "SettingsBinder: failed to write" << settings->fileName();
    foreach (const QPointer<QWidget> &widget, rows)
        if (widget)
            settingsrow::flash(settingsrow::row_of(widget), "saved", ok ? "true" : "error", ok ? saved_ms : error_ms);
    if (ok && saved_callback) saved_callback();
}
