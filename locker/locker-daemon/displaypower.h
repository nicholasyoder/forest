// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISPLAYPOWER_H
#define DISPLAYPOWER_H

#include <QHash>
#include <QObject>

class OutputPowerManager;
class OutputPower;
class QScreen;

// wlr-output-power-management: one zwlr_output_power_v1 per QScreen.
class DisplayPower : public QObject {
    Q_OBJECT
public:
    explicit DisplayPower(QObject *parent = nullptr);
    ~DisplayPower() override;

    bool isValid() const;
    void setAll(bool on);

private:
    void addScreen(QScreen *screen);
    void removeScreen(QScreen *screen);

    OutputPowerManager *m_manager;
    QHash<QScreen *, OutputPower *> m_outputs;
};

#endif // DISPLAYPOWER_H
