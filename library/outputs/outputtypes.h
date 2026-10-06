// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OUTPUTTYPES_H
#define OUTPUTTYPES_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QSize>
#include <QString>

struct OutputModeInfo {
    QSize size;
    int refresh = 0; // mHz, 0 = unknown
    bool preferred = false;
};

// One head as of the last manager `done`.
struct OutputHeadInfo {
    QString name; // connector, e.g. "DP-1"
    QString description;
    QString make;
    QString model;
    QString serial;
    QList<OutputModeInfo> modes;
    int currentMode = -1; // index into modes, -1 when disabled
    bool enabled = false;
    QPoint pos;
    int transform = 0; // wl_output_transform
    double scale = 1.0;
    bool adaptiveSync = false;

    const OutputModeInfo *current() const { return currentMode >= 0 ? &modes[currentMode] : nullptr; }
};

struct OutputState {
    QList<OutputHeadInfo> heads;
    bool adaptiveSyncSupported = false; // manager bound at v4+
};

// Desired settings for one output: an apply request, or one output of a profile.
struct OutputConfig {
    QString connector;
    QString key; // identity key, see identityKeys()
    bool enabled = true;
    QSize size;
    int refresh = 0;
    double scale = 1.0;
    QPoint pos;
    int transform = 0;
    bool adaptiveSync = false;
};

using OutputLayout = QList<OutputConfig>;

namespace outputs {

// "3840x2160@60000" (refresh in mHz).
QString modeToString(QSize size, int refresh);
bool modeFromString(const QString &str, QSize *size, int *refresh);

// wlr-randr names: normal, 90, 180, 270, flipped, flipped-90, ...
QString transformToString(int transform);
int transformFromString(const QString &str);

// Connector -> identity key: make|model|serial when the serial is non-empty and
// unique among the heads, else the connector name.
QHash<QString, QString> identityKeys(const OutputState &state);

// Live layout of every head, including disabled ones.
OutputLayout currentLayout(const OutputState &state);

// True when applying `layout` wouldn't change anything.
bool layoutMatchesState(const OutputLayout &layout, const OutputState &state);

}

#endif // OUTPUTTYPES_H
