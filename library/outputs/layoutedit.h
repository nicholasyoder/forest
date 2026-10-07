// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LAYOUTEDIT_H
#define LAYOUTEDIT_H

#include "outputtypes.h"

// Editor-side layout fixups. Results are always connected and normalized.
namespace layoutedit {

// Moves outputs that don't touch `anchor`'s group next to it, nearest first.
OutputLayout attach(OutputLayout layout, const QString &anchor);

// After `connector` changed size (it was at `before`): outputs beyond its old
// right/bottom edge shift with it, so side-by-side layouts stay side by side.
OutputLayout resized(OutputLayout layout, const QString &connector, const QRect &before);

// Enables `connector` right of the rightmost enabled output.
OutputLayout enable(OutputLayout layout, const QString &connector);

}

#endif // LAYOUTEDIT_H
