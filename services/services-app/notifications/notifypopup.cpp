// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notifypopup.h"

#include <QDialog>
#include <QScrollArea>
#include <QMouseEvent>

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
    body_text = body;
    summary_text = summary;
    bodylabel = new FadingLabel(body);
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
    QTimer::singleShot(0, this, [this]() {  // Run with 0 timer so layout happens and move works with actual widget dimensions
        QRect geo = qApp->primaryScreen()->availableGeometry();
        move(geo.x() + geo.width() - width(), geo.y() + geo.height() - height());
    });

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

void notifypopup::enterEvent(QEnterEvent *){
    resume_timeout = timeout_timer->remainingTime();
    timeout_updater->stop();
    timeout_timer->stop();
    if (bodylabel->is_clipped())
        setCursor(Qt::PointingHandCursor);
}

void notifypopup::leaveEvent(QEvent *){
    timeout_timer->start(resume_timeout);
    timeout_updater->start();
    unsetCursor();
}

/* Open a scrollable detail window if the body text was clipped in the popup */
void notifypopup::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() != Qt::LeftButton || !bodylabel->is_clipped()) return;

    QDialog *detail = new QDialog;
    detail->setAttribute(Qt::WA_DeleteOnClose);
    detail->setWindowTitle(summary_text);
    detail->setProperty("FSS-color", "surface");

    QLabel *header = new QLabel("<b>" + summary_text.toHtmlEscaped() + "</b>");
    header->setWordWrap(true);

    QLabel *body_label = new QLabel(body_text);
    body_label->setWordWrap(true);
    body_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    body_label->setAlignment(Qt::AlignTop);

    QWidget *scroll_contents = new QWidget;
    scroll_contents->setProperty("FSS-color", "pane");

    QVBoxLayout *scroll_layout = new QVBoxLayout(scroll_contents);
    scroll_layout->addWidget(body_label);
    scroll_layout->addStretch(1);

    QScrollArea *scroll = new QScrollArea;
    scroll->setProperty("FSS-widget-type", "pane");
    scroll->setWidget(scroll_contents);
    scroll->setWidgetResizable(true);

    QVBoxLayout *layout = new QVBoxLayout(detail);
    layout->addWidget(header);
    layout->addSpacing(4);
    layout->addWidget(scroll);
    QSize screensize = detail->screen()->availableSize();
    detail->resize(qreal(screensize.width()) * 0.5, qreal(screensize.height()) * 0.75);
    detail->show();
    detail->raise();
    detail->activateWindow();
}
