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

notifypopup::notifypopup(QString app_name, QString summary, QString body, QString app_icon, int timeout, uint id){
    Qt::WindowFlags flags;
    flags |= Qt::WindowStaysOnTopHint;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
    setAutoFillBackground(true);

    QVBoxLayout *contentsvlayout = new QVBoxLayout;
    contentsvlayout->setContentsMargins(QMargins(0,0,0,0));
    contentsvlayout->setSpacing(0);

    QHBoxLayout *tophlayout = new QHBoxLayout;
    tophlayout->setContentsMargins(QMargins(0,0,0,0));
    tophlayout->setSpacing(0);

    QLabel *iconlabel = new QLabel;
    iconlabel->setObjectName("iconLabel");
    iconlabel->setPixmap(geticon(app_icon, app_name).pixmap(32,32, QIcon::Normal, QIcon::On));
    tophlayout->addWidget(iconlabel);

    QVBoxLayout *topvlayout = new QVBoxLayout;
    topvlayout->setContentsMargins(QMargins(0,0,0,0));
    topvlayout->setSpacing(0);

    QLabel *summarylabel = new QLabel(summary);
    summarylabel->setObjectName("summaryLabel");
    topvlayout->addWidget(summarylabel);

    QLabel *appnamelabel = new QLabel(app_name);
    appnamelabel->setObjectName("appnameLabel");
    topvlayout->addWidget(appnamelabel);
    topvlayout->addStretch(1);

    tophlayout->addLayout(topvlayout);
    tophlayout->addStretch(1);

    QPushButton *closebt = new QPushButton;
    closebt->setIcon(QIcon::fromTheme("dialog-close"));
    closebt->setObjectName("closeButton");
    connect(closebt, SIGNAL(clicked()), this, SLOT(close()));
    tophlayout->addWidget(closebt);
    contentsvlayout->addLayout(tophlayout);
    FadingLabel *bodylabel = new FadingLabel(body);
    bodylabel->setObjectName("bodyLabel");
    bodylabel->setWordWrap(true);
    bodylabel->setAlignment(Qt::AlignTop);
    contentsvlayout->addWidget(bodylabel);

    QVBoxLayout *basevlayout = new QVBoxLayout;
    basevlayout->setContentsMargins(QMargins(0,0,0,0));
    basevlayout->setSpacing(0);

    QFrame *contents_box = new QFrame;
    contents_box->setObjectName("contentsBox");
    contents_box->setLayout(contentsvlayout);
    basevlayout->addWidget(contents_box);

    timeout_bar = new NonSegmentedProgressBar;
    basevlayout->addWidget(timeout_bar);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(QMargins(0,0,0,0));
    QFrame *popupQFrame = new QFrame;
    popupQFrame->setObjectName("notifyPopup");
    popupQFrame->setLayout(basevlayout);
    vlayout->addWidget(popupQFrame);

    QSettings settings("Forest", "Forest");
    settings.beginGroup("notifications");
    int min_timeout = settings.value("min_timeout", 3).toInt() * 1000;
    int max_timeout = settings.value("max_timeout", 30).toInt() * 1000;
    int default_timeout = settings.value("default_timeout", 8).toInt() * 1000;
    qreal height_percent = settings.value("height", 0.7).toReal();
    qreal width_percent = settings.value("width", 0.5).toReal();

    QRect screengeo = qApp->primaryScreen()->availableGeometry();
    popupQFrame->setMaximumSize(screengeo.width() * width_percent, screengeo.height() * height_percent);
    move(screengeo.width() - sizeHint().width(), screengeo.height() - sizeHint().height());

    popupid = id;

    if (timeout <= 0) full_timeout = default_timeout;
    else if (timeout < min_timeout) full_timeout = min_timeout;
    else if(timeout > max_timeout) full_timeout = max_timeout;
    else full_timeout = timeout;

    timeout_timer = new QTimer(this);
    timeout_timer->setSingleShot(true);
    connect(timeout_timer, &QTimer::timeout, this, &notifypopup::closepopup);
    timeout_timer->start(full_timeout);

    timeout_updater = new QTimer(this);
    connect(timeout_updater, &QTimer::timeout, this, &notifypopup::update_timeout_bar);
    timeout_updater->start(10);
}

/* Load a QIcon from the icon string or app name provided in the notification message */
QIcon notifypopup::geticon(QString icon_name, QString app_name){
    if(icon_name == "")
        icon_name = app_name;  // if the icon is blank then see if we can get an icon named the same as the app. This works for discord for instance.
    else if(icon_name.startsWith("file://")) // XdgIcon can handle file paths but doesn't seem to know what to do with a file uri
        icon_name.remove("file://");

    return XdgIcon::fromTheme(icon_name, QIcon::fromTheme("unknown"));
}

/* Update the timeout bar to display an indicator of how soon the notification will close */
void notifypopup::update_timeout_bar(){
    timeout_bar->setValue(qreal(timeout_timer->remainingTime()) / qreal(full_timeout));
}
