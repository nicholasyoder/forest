// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGSBINDER_H
#define SETTINGSBINDER_H

#include <QHash>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QWidget>
#include <functional>

class QComboBox;

// Loads and saves controls through their USER property (QSpinBox value, QCheckBox checked,
// QComboBox currentText, ...). Only changed keys are written; saved rows flash "saved".
class SettingsBinder : public QObject {
    Q_OBJECT
public:
    struct Converter {
        std::function<QVariant(const QVariant&)> to_widget;
        std::function<QVariant(const QVariant&)> to_stored;
    };

    SettingsBinder(const QString &organization, const QString &application,
                   const QString &group = QString(), QObject *parent = nullptr);

    void bind(QWidget *widget, const QString &key, const QVariant &default_value,
              const Converter &converter = Converter());
    // Runs after each successful save.
    void set_callback(std::function<void()> callback){ saved_callback = callback; }
    // For hand-written controls: saves through write with the next batch and flashes row's row.
    void changed(QWidget *row, std::function<void(QSettings&)> write);

    // Combo storing item data instead of its text.
    static Converter item_data(QComboBox *combo);
    // Fraction stored, int percent shown.
    static Converter percent();

public slots:
    // Page-level connections still fire while loading; only saving is suppressed.
    void load();
    void flush();

private slots:
    void control_changed();

private:
    struct Binding {
        QString key;
        QVariant default_value;
        Converter converter;
    };

    QSettings *open_settings();
    void schedule(bool immediate);

    QString organization, application, group;
    QHash<QWidget*, Binding> bindings;
    QList<QPointer<QWidget>> dirty;
    QList<QPair<QPointer<QWidget>, std::function<void(QSettings&)>>> writers;
    std::function<void()> saved_callback;
    QTimer timer;
    bool loading = false;
};

#endif // SETTINGSBINDER_H
