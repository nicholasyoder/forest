/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL3+
 *
 * Copyright: 2020 Nicholas Yoder
 * Authors:
 *   Nicholas Yoder <nobody@noemail.com>
 *
 * Modified from elokab systray plugin
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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QPaintEvent>
#include <QSettings>

#include <QX11Info>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xdamage.h>

class TrayIcon: public QWidget
{
    Q_OBJECT

public:
    TrayIcon(Window iconId, QWidget* parent);
    virtual ~TrayIcon();

public slots:
    Window iconId() { return m_iconId; }
    Window windowId() { return m_windowId; }
    bool isValid() const { return m_iconValid; }
    QSize iconSize() const { return m_iconSize; }
    void setIconSize(const QSize &iconSize);
    QSize sizeHint() const;

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void draw(QPaintEvent* event);

private:
    bool init();
    Atom atom(const char* atomName);

    QRect iconGeometry();
    Window m_iconId;
    Window m_windowId;
    bool m_iconValid;
    QSize m_iconSize;
    Damage m_damage;

};
#endif // TRAYICON_H
