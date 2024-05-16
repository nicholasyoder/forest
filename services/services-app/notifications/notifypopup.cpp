/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
 *
 * Copyright: 2021 Nicholas Yoder
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

#include "notifypopup.h"

notifypopup::notifypopup(QString app_name, QString summary, QString body, QString app_icon, int timeout, uint id)
{
    Qt::WindowFlags flags;
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
    setAutoFillBackground(true);

    QHBoxLayout *basehlayout = new QHBoxLayout;
    basehlayout->setMargin(0);

    QVBoxLayout *leftvlayout = new QVBoxLayout;
    QLabel *iconlabel = new QLabel;
    iconlabel->setObjectName("iconLabel");
    iconlabel->setPixmap(geticon(app_icon).pixmap(48,48, QIcon::Normal, QIcon::On));
    leftvlayout->addWidget(iconlabel);
    basehlayout->addLayout(leftvlayout);

    QVBoxLayout *rightvlayout = new QVBoxLayout;
    QHBoxLayout *tophlayout = new QHBoxLayout;
    QLabel *summarylabel = new QLabel(summary);
    summarylabel->setObjectName("summaryLabel");
    tophlayout->addWidget(summarylabel);
    QPushButton *closebt = new QPushButton;
    closebt->setIcon(QIcon::fromTheme("dialog-close"));
    closebt->setObjectName("closeButton");
    connect(closebt, SIGNAL(clicked()), this, SLOT(close()));
    tophlayout->addWidget(closebt);
    rightvlayout->addLayout(tophlayout);
    QLabel *bodylabel = new QLabel(body);
    bodylabel->setObjectName("bodyLabel");
    rightvlayout->addWidget(bodylabel);
    QLabel *appnamelabel = new QLabel("- " + app_name);
    appnamelabel->setObjectName("appnameLabel");
    appnamelabel->setAlignment(Qt::AlignRight);
    rightvlayout->addWidget(appnamelabel);
    basehlayout->addLayout(rightvlayout);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    QFrame *panelQFrame = new QFrame;
    panelQFrame->setObjectName("notifyPopup");
    panelQFrame->setLayout(basehlayout);
    vlayout->addWidget(panelQFrame);

    QRect screengeo = qApp->primaryScreen()->availableGeometry();
    move(screengeo.width() - sizeHint().width(), screengeo.height() - sizeHint().height());

    popupid = id;

    if (timeout > 0 && timeout < 3000) timeout = 3000;
    else if(timeout <= 0 || timeout > 8000) timeout = 8000;

    QTimer::singleShot(timeout, this, SLOT(closepopup()));
}

QIcon notifypopup::geticon(QString icon){
    QIcon ico;
    if (QIcon::hasThemeIcon(icon)){ico = QIcon::fromTheme(icon);}
    else{
        if (icon.startsWith("/")){ico = QIcon(icon);}
        else{
            QFile icofile("/usr/share/pixmaps/" + icon + ".png");
            if (icofile.exists()){ico = QIcon("/usr/share/pixmaps/" + icon + ".png");}
            else{
                QFile icofile2("/usr/share/pixmaps/" + icon);
                if (icofile2.exists()){ico = QIcon("/usr/share/pixmaps/" + icon);}
                else{
                    QFile icofile3("/usr/share/pixmaps/" + icon + ".svg");
                    if (icofile3.exists()){ico = QIcon("/usr/share/pixmaps/" + icon + ".svg");}
                    else{ico = XdgIcon::fromTheme(icon, QIcon::fromTheme("unknown"));}
                }
            }
        }
    }
    return ico;
}
