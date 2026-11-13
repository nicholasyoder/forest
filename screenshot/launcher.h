// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QDialog>

class QPushButton;
class QCheckBox;
class QSpinBox;

// Capture UI shown when no mode is given. Clicking a mode button hides it and
// emits captureRequested(); options persist in Forest.conf ([screenshot]).
class Launcher : public QDialog {
    Q_OBJECT

public:
    // Values match the radio ids; main.cpp maps them to its own Mode.
    enum Mode { Full, Monitor, Window, Region };

    explicit Launcher(QWidget *parent = nullptr);

signals:
    void captureRequested(int mode, int delay, bool save, bool clipboard);

private:
    void trigger(Mode mode);

    QPushButton *m_monitor;
    QSpinBox *m_delay;
    QCheckBox *m_save;
    QCheckBox *m_clipboard;
};

#endif // LAUNCHER_H
