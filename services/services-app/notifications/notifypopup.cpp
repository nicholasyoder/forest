// SPDX-License-Identifier: LGPL-3.0-or-later

#include "notifypopup.h"

#include <QDialog>
#include <QScrollArea>
#include <QMouseEvent>
#include <QWindow>
#include <QUrl>

#include <LayerShellQt/Window>

#include "miscutills.h"
#include "../../notificationsconfig.h"

namespace {
// Spec body-markup -> safe Qt rich text: only b/i/u/a(http, https, mailto)/br survive, everything else is escaped or dropped.
QString sanitize_markup(const QString &body){
    static const QRegularExpression tag("<(/?)(b|i|u|a|br|img)\\b([^>]*?)(/?)>", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression href("\\bhref\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')");
    auto text = [](QString t) {
        t.replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'").replace("&amp;", "&");
        return t.toHtmlEscaped().replace('\n', "<br>");
    };
    QString out;
    int pos = 0;
    auto it = tag.globalMatch(body);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out += text(body.mid(pos, m.capturedStart() - pos));
        pos = m.capturedEnd();
        const QString name = m.captured(2).toLower();
        const bool closing = !m.captured(1).isEmpty();
        if (name == "img") continue;
        if (name == "br") { out += "<br>"; continue; }
        if (name == "a" && !closing) {
            const QRegularExpressionMatch h = href.match(m.captured(3));
            QString url = h.hasMatch() ? (h.captured(1).isEmpty() ? h.captured(2) : h.captured(1)) : QString();
            url.replace("&amp;", "&");
            const QString scheme = QUrl(url).scheme().toLower();
            if (scheme == "http" || scheme == "https" || scheme == "mailto") {
                out += "<a href=\"" + url.toHtmlEscaped() + "\">";
                continue;
            }
            out += "<a>"; // keeps </a> balanced
            continue;
        }
        out += closing ? "</" + name + ">" : "<" + name + ">";
    }
    return out + text(body.mid(pos));
}
}

notifypopup::notifypopup(QString app_name, QString summary, QString body, QString app_icon, const QStringList &actions, int timeout, uint id){
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(true);

    QVBoxLayout *contentsvlayout = new QVBoxLayout;
    contentsvlayout->setContentsMargins(QMargins(0,0,0,0));
    contentsvlayout->setSpacing(0);

    QHBoxLayout *tophlayout = new QHBoxLayout;
    tophlayout->setContentsMargins(QMargins(0,0,0,0));
    tophlayout->setSpacing(0);

    iconlabel = new QLabel;
    iconlabel->setObjectName("iconLabel");
    iconlabel->setPixmap(geticon(app_icon, app_name).pixmap(32,32, QIcon::Normal, QIcon::On));
    tophlayout->addWidget(iconlabel);

    QVBoxLayout *topvlayout = new QVBoxLayout;
    topvlayout->setContentsMargins(QMargins(0,0,0,0));
    topvlayout->setSpacing(0);

    summarylabel = new QLabel(summary);
    summarylabel->setTextFormat(Qt::PlainText); // body-markup isn't advertised
    summarylabel->setObjectName("summaryLabel");
    topvlayout->addWidget(summarylabel);

    appnamelabel = new QLabel(app_name);
    appnamelabel->setTextFormat(Qt::PlainText);
    appnamelabel->setObjectName("appnameLabel");
    topvlayout->addWidget(appnamelabel);
    topvlayout->addStretch(1);

    tophlayout->addLayout(topvlayout);
    tophlayout->addStretch(1);

    QPushButton *closebt = new QPushButton;
    closebt->setIcon(QIcon::fromTheme("dialog-close"));
    closebt->setObjectName("closeButton");
    connect(closebt, &QPushButton::clicked, this, [this]{ closepopup(2); }); // spec reason: dismissed
    tophlayout->addWidget(closebt);
    contentsvlayout->addLayout(tophlayout);
    body_html = sanitize_markup(body);
    summary_text = summary;
    bodylabel = new FadingLabel(body_html);
    bodylabel->setTextFormat(Qt::RichText);
    bodylabel->setOpenExternalLinks(true);
    bodylabel->setObjectName("bodyLabel");
    bodylabel->setWordWrap(true);
    bodylabel->setAlignment(Qt::AlignTop);
    contentsvlayout->addWidget(bodylabel);

    actionsbox = new QFrame;
    actionsbox->setObjectName("actionsBox");
    actionslayout = new QHBoxLayout(actionsbox);
    actionslayout->setContentsMargins(QMargins(0,0,0,0));
    contentsvlayout->addWidget(actionsbox);
    set_actions(actions);

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
    settings.beginGroup(notificationsconfig::group);
    min_timeout = settings.value(notificationsconfig::min_timeout, notificationsconfig::min_timeout_default).toInt() * 1000;
    max_timeout = qMax(min_timeout, settings.value(notificationsconfig::max_timeout, notificationsconfig::max_timeout_default).toInt() * 1000);
    default_timeout = settings.value(notificationsconfig::default_timeout, notificationsconfig::default_timeout_default).toInt() * 1000;
    qreal height_percent = settings.value(notificationsconfig::height, notificationsconfig::height_default).toReal();
    qreal width_percent = settings.value(notificationsconfig::width, notificationsconfig::width_default).toReal();

    QScreen *screen = ScreenTracker::primary();
    if (!screen)
        screen = qApp->primaryScreen();
    QRect screengeo = screen->availableGeometry();
    popupQFrame->setMaximumSize(screengeo.width() * width_percent, screengeo.height() * height_percent);

    winId(); // force native window creation so windowHandle() is valid
    windowHandle()->setScreen(screen);
    const QString position = settings.value(notificationsconfig::position, notificationsconfig::position_default).toString();
    anchor_top = position.startsWith("top");
    anchor_left = position.endsWith("left");
    layer_window = LayerShellQt::Window::get(windowHandle());
    layer_window->setLayer(LayerShellQt::Window::LayerOverlay);
    layer_window->setAnchors(LayerShellQt::Window::Anchors(
        (anchor_top ? LayerShellQt::Window::AnchorTop : LayerShellQt::Window::AnchorBottom)
        | (anchor_left ? LayerShellQt::Window::AnchorLeft : LayerShellQt::Window::AnchorRight)));
    layer_window->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    layer_window->setScope("forest-notification");

    popupid = id;

    timeout_anim = new QVariantAnimation(this);
    timeout_anim->setStartValue(1.0);
    timeout_anim->setEndValue(0.0);
    connect(timeout_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){ timeout_bar->setValue(v.toReal()); });
    connect(timeout_anim, &QVariantAnimation::finished, this, [this]{
        if (timeout_anim->currentTime() >= timeout_anim->duration()) closepopup(1); // not a stop(); spec reason: expired
    });
    apply_timeout(timeout);
    overlap = read_stack_overlap();
}

void notifypopup::set_actions(const QStringList &actions){
    // Drop the old buttons, keeping the leading stretch spacer out of the way.
    while (QLayoutItem *item = actionslayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    actionslayout->addStretch(1);
    has_default_action = false;
    // actions is a flat [key, label, key, label, ...] list; "default" is a click on the popup itself.
    for (int i = 0; i + 1 < actions.size(); i += 2) {
        const QString key = actions[i];
        if (key == "default") {
            has_default_action = true;
            continue;
        }
        QPushButton *actionbt = new QPushButton(actions[i + 1]);
        actionbt->setObjectName("actionButton");
        connect(actionbt, &QPushButton::clicked, this, [this, key]{ emit actionInvoked(popupid, key); });
        actionslayout->addWidget(actionbt);
    }
    actionsbox->setVisible(actionslayout->count() > 1);
}

// timeout is the spec's expire_timeout: -1 server default, 0 never, else clamped to [min, max].
void notifypopup::apply_timeout(int timeout){
    if (timeout < 0) full_timeout = default_timeout;
    else if (timeout == 0) full_timeout = 0;
    else full_timeout = qBound(min_timeout, timeout, max_timeout);

    timeout_anim->stop();
    timeout_bar->setVisible(full_timeout > 0);
    if (full_timeout > 0) {
        timeout_bar->setValue(1);
        timeout_anim->setDuration(full_timeout);
        if (started && !underMouse())
            timeout_anim->start();
    }
}

void notifypopup::update_content(const QString &app_name, const QString &summary, const QString &body, const QString &app_icon,
                                 const QStringList &actions, int timeout){
    summary_text = summary;
    body_html = sanitize_markup(body);
    summarylabel->setText(summary);
    appnamelabel->setText(app_name);
    bodylabel->setText(body_html);
    iconlabel->setPixmap(geticon(app_icon, app_name).pixmap(32,32, QIcon::Normal, QIcon::On));
    set_actions(actions);
    apply_timeout(timeout);
    if (underMouse() && full_timeout > 0) { // the pointer hold applies to the new timeout too
        timeout_anim->start();
        timeout_anim->pause();
    }
}

void notifypopup::set_stack_offset(int offset){
    QMargins margins;
    (anchor_top ? margins.setTop(offset) : margins.setBottom(offset));
    layer_window->setMargins(margins);
}

int notifypopup::read_stack_overlap() const{
    // Last matching rule wins; handles "margin: a [b [c [d]]]" and margin-top/bottom.
    const QString side = anchor_top ? "bottom" : "top";
    int overlap = 0;
    QRegularExpression rule("#notifyPopup\\b[^{}]*\\{([^}]*)\\}");
    QRegularExpressionMatchIterator it = rule.globalMatch(qApp->styleSheet());
    while (it.hasNext()) {
        for (const QString &decl : it.next().captured(1).split(';')) {
            const QString key = decl.section(':', 0, 0).trimmed();
            QStringList v = decl.section(':', 1).simplified().split(' ', Qt::SkipEmptyParts);
            if (v.isEmpty()) continue;
            if (key == "margin") { // top is the 1st value; bottom the 3rd, else the 1st/2nd
                const int i = (side == "bottom" && v.size() > 2) ? 2 : 0;
                overlap = v.value(i).remove("px").toInt();
            } else if (key == "margin-" + side) {
                overlap = v.first().remove("px").toInt();
            }
        }
    }
    return overlap;
}

// Timers start when mapped: popups can be held back during startup.
void notifypopup::showEvent(QShowEvent *){
    if (started) return;
    started = true;
    if (full_timeout > 0)
        timeout_anim->start();
}

void notifypopup::resizeEvent(QResizeEvent *){
    emit heightChanged();
}

/* Load a QIcon from the icon string or app name provided in the notification message */
QIcon notifypopup::geticon(QString icon_name, QString app_name){
    if(icon_name == "")
        icon_name = app_name;  // if the icon is blank then see if we can get an icon named the same as the app. This works for discord for instance.
    else if(icon_name.startsWith("file://")) // XdgIcon can handle file paths but doesn't seem to know what to do with a file uri
        icon_name.remove("file://");

    return XdgIcon::fromTheme(icon_name, QIcon::fromTheme("unknown"));
}

void notifypopup::enterEvent(QEnterEvent *){
    if (timeout_anim->state() == QAbstractAnimation::Running)
        timeout_anim->pause();
    if (has_default_action || bodylabel->is_clipped())
        setCursor(Qt::PointingHandCursor);
}

void notifypopup::leaveEvent(QEvent *){
    if (timeout_anim->state() == QAbstractAnimation::Paused)
        timeout_anim->resume();
    unsetCursor();
}

/* Open a scrollable detail window if the body text was clipped in the popup */
void notifypopup::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() != Qt::LeftButton) return;
    if (has_default_action) {
        emit actionInvoked(popupid, "default");
        return;
    }
    if (!bodylabel->is_clipped()) return;

    QDialog *detail = new QDialog;
    detail->setAttribute(Qt::WA_DeleteOnClose);
    detail->setWindowTitle(summary_text);
    detail->setProperty("FSS-color", "surface");

    QLabel *header = new QLabel("<b>" + summary_text.toHtmlEscaped() + "</b>");
    header->setWordWrap(true);

    QLabel *body_label = new QLabel(body_html);
    body_label->setTextFormat(Qt::RichText);
    body_label->setOpenExternalLinks(true);
    body_label->setWordWrap(true);
    body_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
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
