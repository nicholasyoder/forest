// SPDX-License-Identifier: LGPL-3.0-or-later

#include "launcher.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QGuiApplication>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr auto kGroup = "screenshot";
}

Launcher::Launcher(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Screenshot"));

    QSettings s("Forest", "Forest");
    s.beginGroup(kGroup);

    const struct { Mode mode; QString label; } entries[] = {
        {Full, tr("Whole workspace")},
        {Monitor, tr("Single monitor")},
        {Window, tr("Active window")},
        {Region, tr("Region")},
    };
    auto *modes = new QVBoxLayout;
    QList<QPushButton *> buttons;
    for (const auto &e : entries) {
        auto *b = new QPushButton(e.label, this);
        if (e.mode == Monitor)
            m_monitor = b; // created in order so tab order follows the layout
        connect(b, &QPushButton::clicked, this, [=] { trigger(e.mode); });
        modes->addWidget(b);
        buttons.append(b);
    }
    // One output makes "single monitor" identical to the whole workspace.
    auto syncMonitor = [=] { m_monitor->setVisible(QGuiApplication::screens().size() > 1); };
    connect(qGuiApp, &QGuiApplication::screenAdded, this, syncMonitor);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, syncMonitor);
    syncMonitor();

    m_delay = new QSpinBox(this);
    m_delay->setRange(0, 60);
    m_delay->setSuffix(tr(" s"));
    m_delay->setValue(s.value("delay", 0).toInt());
    m_save = new QCheckBox(tr("Save to Pictures/Screenshots"), this);
    m_save->setChecked(s.value("save", true).toBool());
    m_clipboard = new QCheckBox(tr("Copy to clipboard"), this);
    m_clipboard->setChecked(s.value("clipboard", true).toBool());

    // With nothing to deliver to, capturing is pointless.
    auto syncEnabled = [=] {
        for (QPushButton *b : buttons)
            b->setEnabled(m_save->isChecked() || m_clipboard->isChecked());
    };
    connect(m_save, &QCheckBox::toggled, this, syncEnabled);
    connect(m_clipboard, &QCheckBox::toggled, this, syncEnabled);
    syncEnabled();

    auto *form = new QFormLayout;
    form->addRow(tr("Delay:"), m_delay);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(modes);
    layout->addLayout(form);
    layout->addWidget(m_save);
    layout->addWidget(m_clipboard);
}

void Launcher::trigger(Mode mode) {
    QSettings s("Forest", "Forest");
    s.beginGroup(kGroup);
    s.setValue("delay", m_delay->value());
    s.setValue("save", m_save->isChecked());
    s.setValue("clipboard", m_clipboard->isChecked());
    s.endGroup();
    hide();
    emit captureRequested(mode, m_delay->value(), m_save->isChecked(), m_clipboard->isChecked());
}
