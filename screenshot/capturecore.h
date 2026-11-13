// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CAPTURECORE_H
#define CAPTURECORE_H

#include <QImage>
#include <QList>
#include <QString>

class QScreen;

namespace capturecore {

struct Options {
    bool save = true;
    bool clipboard = true;
    QString outputFile; // overrides the auto-named path in the Screenshots dir
};

struct Shot {
    QScreen *screen;
    QImage image;
};

// Composites per-output grabs by logical geometry at the densest output's
// scale, so fractional-scale captures keep their physical pixels.
QImage compose(const QList<Shot> &shots);

// Next free Screenshot_YYYY-MM-DD_HH-MM-SS[_N].png in <Pictures>/Screenshots
// (created if missing); empty on failure.
QString autoPath();

// Saves and/or copies per `opts`, then notifies. Returns the process exit code.
int deliver(const QImage &image, const Options &opts);

void notify(const QString &summary, const QString &body, bool failure);

} // namespace capturecore

#endif // CAPTURECORE_H
