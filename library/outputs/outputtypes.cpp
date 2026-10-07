// SPDX-License-Identifier: LGPL-3.0-or-later

#include "outputtypes.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>

#include <wayland-util.h>

#include <algorithm>
#include <climits>
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

QSize effectiveSize(const OutputConfig &config){
    QSize size = config.transform & 1 ? config.size.transposed() : config.size;
    float scale = wl_fixed_to_double(wl_fixed_from_double(config.scale));
    if (scale <= 0) scale = 1;
    return QSize(int(size.width() / scale), int(size.height() / scale));
}

QRect layoutRect(const OutputConfig &config){
    return QRect(config.pos, effectiveSize(config));
}

bool isConnected(const OutputLayout &layout){
    QList<QRect> rects;
    for (const OutputConfig &config : layout)
        if (config.enabled) rects << layoutRect(config);
    if (rects.isEmpty()) return false;

    // Same predicate as Biome's output_arrange.cpp: positive overlap on one axis,
    // at least touching on the other.
    auto touches = [](const QRect &a, const QRect &b){
        const int ow = std::min(a.x() + a.width(), b.x() + b.width()) - std::max(a.x(), b.x());
        const int oh = std::min(a.y() + a.height(), b.y() + b.height()) - std::max(a.y(), b.y());
        return (ow > 0 && oh >= 0) || (oh > 0 && ow >= 0);
    };

    QList<bool> reached(rects.size(), false);
    QList<int> pending = {0};
    reached[0] = true;
    while (!pending.isEmpty()){
        const QRect a = rects[pending.takeLast()];
        for (int i = 0; i < rects.size(); i++){
            if (!reached[i] && touches(a, rects[i])){
                reached[i] = true;
                pending << i;
            }
        }
    }
    return !reached.contains(false);
}

OutputLayout normalized(OutputLayout layout){
    int minX = INT_MAX, minY = INT_MAX;
    for (const OutputConfig &config : layout){
        if (!config.enabled) continue;
        minX = std::min(minX, config.pos.x());
        minY = std::min(minY, config.pos.y());
    }
    if (minX == INT_MAX) return layout;
    for (OutputConfig &config : layout)
        if (config.enabled) config.pos -= QPoint(minX, minY);
    return layout;
}

QString topLeft(const OutputLayout &layout){
    const OutputConfig *best = nullptr;
    for (const OutputConfig &config : layout){
        if (!config.enabled) continue;
        if (!best || config.pos.x() < best->pos.x()
            || (config.pos.x() == best->pos.x() && config.pos.y() < best->pos.y()))
            best = &config;
    }
    return best ? best->connector : QString();
}

bool needsConfirm(const OutputLayout &before, const OutputLayout &after){
    for (const OutputConfig &a : after){
        auto b = std::find_if(before.begin(), before.end(),
                              [&](const OutputConfig &c){ return c.connector == a.connector; });
        if (b == before.end() || a.enabled != b->enabled) return true;
        if (!a.enabled) continue;
        if (a.size != b->size || a.refresh != b->refresh || a.transform != b->transform
            || !sameScale(a.scale, b->scale))
            return true;
    }
    return false;
}

QString layoutToJson(const OutputLayout &layout, const QString &primary){
    QJsonArray array;
    for (const OutputConfig &config : layout){
        array.append(QJsonObject{
            {"connector", config.connector},
            {"enabled", config.enabled},
            {"mode", modeToString(config.size, config.refresh)},
            {"scale", config.scale},
            {"x", config.pos.x()},
            {"y", config.pos.y()},
            {"transform", transformToString(config.transform)},
            {"adaptive_sync", config.adaptiveSync},
        });
    }
    const QJsonObject object{{"outputs", array}, {"primary", primary}};
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

bool layoutFromJson(const QString &json, OutputLayout *layout, QString *primary){
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) return false;
    OutputLayout result;
    for (const QJsonValue &value : doc.object()["outputs"].toArray()){
        const QJsonObject object = value.toObject();
        OutputConfig config;
        config.connector = object["connector"].toString();
        if (config.connector.isEmpty()) return false;
        config.enabled = object["enabled"].toBool(true);
        if (!modeFromString(object["mode"].toString(), &config.size, &config.refresh)) return false;
        config.scale = object["scale"].toDouble(1.0);
        if (config.scale <= 0) return false;
        config.pos = QPoint(object["x"].toInt(), object["y"].toInt());
        config.transform = transformFromString(object["transform"].toString());
        config.adaptiveSync = object["adaptive_sync"].toBool();
        result << config;
    }
    *layout = result;
    *primary = doc.object()["primary"].toString();
    return true;
}

}
