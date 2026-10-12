// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYPOWER_H
#define DISPLAYPOWER_H

#include <QHash>
#include <QObject>

class OutputPowerManager;
class OutputPower;
class QScreen;

// wlr-output-power-management: one zwlr_output_power_v1 per QScreen. Exports the
// blank state read-only on org.forest.DisplayPower (power control is exclusive per output).
class DisplayPower : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.forest.DisplayPower")
public:
    explicit DisplayPower(QObject *parent = nullptr);
    ~DisplayPower() override;

    bool isValid() const;
    void setAll(bool on);

public slots:
    Q_SCRIPTABLE bool DisplaysOff() const { return !m_on; }

signals:
    Q_SCRIPTABLE void DisplaysOffChanged(bool off);

private:
    void addScreen(QScreen *screen);
    void removeScreen(QScreen *screen);

    OutputPowerManager *m_manager;
    QHash<QScreen *, OutputPower *> m_outputs;
    bool m_on = true;
};

#endif // DISPLAYPOWER_H
