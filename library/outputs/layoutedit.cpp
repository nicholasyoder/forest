// SPDX-License-Identifier: LGPL-3.0-or-later

#include "layoutedit.h"

#include <algorithm>
#include <climits>
#include <cstdlib>

namespace {

int indexOf(const OutputLayout &layout, const QString &connector){
    for (int i = 0; i < layout.size(); i++)
        if (layout[i].connector == connector) return i;
    return -1;
}

// Indices of the enabled outputs edge-connected to `start` (Biome's rule).
QList<int> group(const OutputLayout &layout, int start){
    QList<int> result = {start};
    for (int i = 0; i < result.size(); i++){
        for (int j = 0; j < layout.size(); j++){
            if (!layout[j].enabled || result.contains(j)) continue;
            if (outputs::isConnected({layout[result[i]], layout[j]})) result << j;
        }
    }
    return result;
}

}

namespace layoutedit {

OutputLayout attach(OutputLayout layout, const QString &anchor){
    int start = indexOf(layout, anchor);
    if (start < 0 || !layout[start].enabled){
        start = -1;
        for (int i = 0; i < layout.size() && start < 0; i++)
            if (layout[i].enabled) start = i;
        if (start < 0) return layout;
    }

    for (int round = 0; round < layout.size(); round++){
        const QList<int> attached = group(layout, start);
        QList<QRect> groupRects;
        for (int i : attached) groupRects << outputs::layoutRect(layout[i]);

        int bestIndex = -1, bestDistance = INT_MAX;
        QPoint bestPos;
        for (int i = 0; i < layout.size(); i++){
            if (!layout[i].enabled || attached.contains(i)) continue;
            const QRect o = outputs::layoutRect(layout[i]);
            for (const QRect &g : std::as_const(groupRects)){
                // Touch one of g's sides, keeping a positive shared edge.
                const int y = std::clamp(o.y(), g.y() - o.height() + 1, g.y() + g.height() - 1);
                const int x = std::clamp(o.x(), g.x() - o.width() + 1, g.x() + g.width() - 1);
                const QList<QPoint> candidates = {
                    {g.x() + g.width(), y}, {g.x() - o.width(), y},
                    {x, g.y() + g.height()}, {x, g.y() - o.height()},
                };
                for (const QPoint &pos : candidates){
                    const QRect moved(pos, o.size());
                    bool clash = false;
                    for (const QRect &other : std::as_const(groupRects))
                        if (moved.intersects(other)) clash = true; // positive-area overlap only
                    // Prefer spots that don't cover another output.
                    const int distance = std::abs(pos.x() - o.x()) + std::abs(pos.y() - o.y()) + (clash ? INT_MAX / 4 : 0);
                    if (distance < bestDistance){
                        bestDistance = distance;
                        bestIndex = i;
                        bestPos = pos;
                    }
                }
            }
        }
        if (bestIndex < 0) break;
        layout[bestIndex].pos = bestPos;
    }
    return outputs::normalized(layout);
}

OutputLayout resized(OutputLayout layout, const QString &connector, const QRect &before){
    const int index = indexOf(layout, connector);
    if (index < 0) return layout;
    const QRect after = outputs::layoutRect(layout[index]);
    const int dw = after.width() - before.width();
    const int dh = after.height() - before.height();
    for (int i = 0; i < layout.size(); i++){
        if (i == index || !layout[i].enabled) continue;
        if (layout[i].pos.x() >= before.x() + before.width()) layout[i].pos.rx() += dw;
        if (layout[i].pos.y() >= before.y() + before.height()) layout[i].pos.ry() += dh;
    }
    return attach(layout, connector);
}

OutputLayout enable(OutputLayout layout, const QString &connector){
    const int index = indexOf(layout, connector);
    if (index < 0) return layout;
    int right = 0, top = INT_MAX;
    bool any = false;
    for (const OutputConfig &config : std::as_const(layout)){
        if (!config.enabled) continue;
        const QRect r = outputs::layoutRect(config);
        if (!any || r.x() + r.width() > right){
            right = r.x() + r.width();
            top = r.y();
        }
        any = true;
    }
    layout[index].enabled = true;
    layout[index].pos = any ? QPoint(right, top) : QPoint(0, 0);
    return attach(layout, connector);
}

}
