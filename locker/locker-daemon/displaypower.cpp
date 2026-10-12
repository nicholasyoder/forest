// SPDX-License-Identifier: LGPL-3.0-or-later

#include "displaypower.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QWaylandClientExtension>

#include <QtWaylandClient/private/qwaylandscreen_p.h>

#include "qwayland-wlr-output-power-management-unstable-v1.h"

class OutputPowerManager : public QWaylandClientExtensionTemplate<OutputPowerManager>,
                           public QtWayland::zwlr_output_power_manager_v1 {
public:
    OutputPowerManager() : QWaylandClientExtensionTemplate<OutputPowerManager>(1) { initialize(); }
};

class OutputPower : public QtWayland::zwlr_output_power_v1 {
public:
    using QtWayland::zwlr_output_power_v1::zwlr_output_power_v1;
    ~OutputPower() override
    {
        if (object())
            destroy();
    }

    void setOn(bool on)
    {
        if (object())
            set_mode(on ? mode_on : mode_off);
    }

protected:
    // The output is gone or can't do power management; requests after this are ignored.
    void zwlr_output_power_v1_failed() override { destroy(); }
};

DisplayPower::DisplayPower(QObject *parent)
    : QObject(parent)
    , m_manager(new OutputPowerManager)
{
    m_manager->setParent(this);
    if (!isValid()) {
        qWarning() << "Compositor lacks wlr-output-power-management: display power disabled";
        return;
    }
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.registerObject(QStringLiteral("/org/forest/DisplayPower"), this,
                       QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    if (!bus.registerService(QStringLiteral("org.forest.DisplayPower")))
        qWarning() << "Couldn't own org.forest.DisplayPower:" << bus.lastError().message();
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &DisplayPower::addScreen);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &DisplayPower::removeScreen);
    for (QScreen *screen : QGuiApplication::screens())
        addScreen(screen);
}

DisplayPower::~DisplayPower()
{
    qDeleteAll(m_outputs);
}

bool DisplayPower::isValid() const
{
    return m_manager->isActive();
}

void DisplayPower::setAll(bool on)
{
    if (!isValid())
        return;
    const bool changed = on != m_on;
    m_on = on;
    for (OutputPower *output : std::as_const(m_outputs))
        output->setOn(on);
    if (changed)
        emit DisplaysOffChanged(!on);
}

void DisplayPower::addScreen(QScreen *screen)
{
    // Not a QWaylandScreen: the placeholder Qt uses while no output exists.
    auto *waylandScreen = dynamic_cast<QtWaylandClient::QWaylandScreen *>(screen->handle());
    if (!waylandScreen || m_outputs.contains(screen))
        return;
    auto *output = new OutputPower(m_manager->get_output_power(waylandScreen->output()));
    // A monitor plugged in while blanked stays dark until the wake.
    if (!m_on)
        output->setOn(false);
    m_outputs.insert(screen, output);
}

void DisplayPower::removeScreen(QScreen *screen)
{
    delete m_outputs.take(screen);
}
