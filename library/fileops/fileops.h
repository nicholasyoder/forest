// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILEOPS_H
#define FILEOPS_H

#include <QList>
#include <QStringList>
#include <QUrl>

// File operations with progress, conflict and error UI. All return immediately.
namespace fileops {

void copy(const QStringList &sources, const QString &destDir);
void move(const QStringList &sources, const QString &destDir);
// Offers a permanent delete for anything that can't be trashed.
void trash(const QStringList &paths);
// Asks for confirmation first.
void remove(const QStringList &paths);

// Writes gnome-copied-files, uri-list and KDE cut-selection formats.
void setClipboard(const QStringList &paths, bool cut);
void paste(const QString &destDir);

// Move within a drive, copy across drives; Ctrl forces copy, Shift forces move.
Qt::DropAction dropAction(const QStringList &sources, const QString &destDir,
                          Qt::KeyboardModifiers modifiers, Qt::DropActions allowed);
QStringList localPaths(const QList<QUrl> &urls);

bool sameDevice(const QString &path, const QString &dir);
// Free "name (tag).ext" path in dir; numbered "(2)", "(3)"... if tag is empty.
QString uniquePath(const QString &dir, const QString &name, const QString &tag);
void showErrors(const QString &text, const QStringList &errors);

}
#endif // FILEOPS_H
