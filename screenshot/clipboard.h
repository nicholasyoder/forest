// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QByteArray>

namespace clipboard {

// Spawns a detached helper (this binary, --serve-clipboard) that owns the
// image/png selection until another client replaces it, like `wl-copy`.
// False if it couldn't be started.
bool copyPng(const QByteArray &png);

// Helper entry point: reads the PNG from stdin and serves it via
// wlr-data-control. Returns the process exit code.
int serve();

} // namespace clipboard

#endif // CLIPBOARD_H
