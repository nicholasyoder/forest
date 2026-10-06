// SPDX-License-Identifier: LGPL-3.0-or-later

#include "outputtypes.h"

#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <cmath>

namespace {
const QStringList kTransforms = {"normal", "90", "180", "270",
                                 "flipped", "flipped-90", "flipped-180", "flipped-270"};

// Protocol scale is wl_fixed (1/256).
bool sameScale(double a, double b){
    return std::abs(a - b) < 1.0 / 256;
}
}

namespace outputs {

QString modeToString(QSize size, int refresh){
    return QString("%1x%2@%3").arg(size.width()).arg(size.height()).arg(refresh);
}

bool modeFromString(const QString &str, QSize *size, int *refresh){
    static const QRegularExpression re("^(\\d+)x(\\d+)@(\\d+)$");
    QRegularExpressionMatch m = re.match(str);
    if (!m.hasMatch()) return false;
    *size = QSize(m.captured(1).toInt(), m.captured(2).toInt());
    *refresh = m.captured(3).toInt();
    return true;
}

QString transformToString(int transform){
    return kTransforms.value(transform, "normal");
}

int transformFromString(const QString &str){
    return qMax(0, kTransforms.indexOf(str));
}

QHash<QString, QString> identityKeys(const OutputState &state){
    QHash<QString, int> serialCount;
    for (const OutputHeadInfo &head : state.heads)
        if (!head.serial.isEmpty()) serialCount[head.serial]++;

    QHash<QString, QString> keys;
    for (const OutputHeadInfo &head : state.heads){
        if (!head.serial.isEmpty() && serialCount[head.serial] == 1)
            keys[head.name] = head.make + "|" + head.model + "|" + head.serial;
        else
            keys[head.name] = head.name;
    }
    return keys;
}

OutputLayout currentLayout(const OutputState &state){
    const QHash<QString, QString> keys = identityKeys(state);
    OutputLayout layout;
    for (const OutputHeadInfo &head : state.heads){
        OutputConfig config;
        config.connector = head.name;
        config.key = keys[head.name];
        config.enabled = head.enabled;
        // Disabled heads have no current mode; remember the preferred one.
        const OutputModeInfo *mode = head.current();
        if (!mode){
            for (const OutputModeInfo &m : head.modes)
                if (m.preferred) mode = &m;
            if (!mode && !head.modes.isEmpty()) mode = &head.modes.first();
        }
        if (mode){
            config.size = mode->size;
            config.refresh = mode->refresh;
        }
        config.scale = head.scale;
        config.pos = head.pos;
        config.transform = head.transform;
        config.adaptiveSync = head.adaptiveSync;
        layout << config;
    }
    return layout;
}

bool layoutMatchesState(const OutputLayout &layout, const OutputState &state){
    if (layout.size() != state.heads.size()) return false;
    for (const OutputHeadInfo &head : state.heads){
        auto it = std::find_if(layout.begin(), layout.end(),
                               [&](const OutputConfig &c){ return c.connector == head.name; });
        if (it == layout.end() || it->enabled != head.enabled) return false;
        if (!head.enabled) continue;
        const OutputModeInfo *mode = head.current();
        if (!mode || mode->size != it->size || mode->refresh != it->refresh) return false;
        if (head.pos != it->pos || head.transform != it->transform || !sameScale(head.scale, it->scale))
            return false;
        if (state.adaptiveSyncSupported && head.adaptiveSync != it->adaptiveSync) return false;
    }
    return true;
}

}
