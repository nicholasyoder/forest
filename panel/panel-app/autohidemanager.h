/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2024 Nicholas Yoder
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef AUTOHIDEMANAGER_H
#define AUTOHIDEMANAGER_H

#include <QDateTime>
#include <QTimer>

class AutoHideManager : public QObject {
    Q_OBJECT
public:
    explicit AutoHideManager(QObject *parent = nullptr);
    ~AutoHideManager();

    bool eventFilter(QObject* obj, QEvent* event) override;

    // Also called from panel on settings reload to start autohide
    void close_eventually();

signals:
    void show();
    void hide();

private slots:
    void check_focus();
    void maybe_close();

private:
    QTimer *check_focus_timer = nullptr;
    QTimer *maybe_close_timer = nullptr;
    QDateTime lost_focus_time;
};

#endif // AUTOHIDEMANAGER_H
