// SPDX-License-Identifier: LGPL-3.0-or-later

#include "trayicon.h"
#include "panelanchor.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDirIterator>
#include <QImage>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QtEndian>
#include <QWheelEvent>

#include <dbusmenuimporter.h>

namespace {

constexpr char kItemInterface[] = "org.kde.StatusNotifierItem";

// "(iiay)": ARGB32 in network byte order, not Qt's native-endian ARGB32.
struct StatusNotifierIconPixmap {
    int width = 0;
    int height = 0;
    QByteArray data;
};

// Must share a namespace with the struct so QList<T>'s operator>> finds it via ADL.
const QDBusArgument &operator>>(const QDBusArgument &arg, StatusNotifierIconPixmap &icon) {
    arg.beginStructure();
    arg >> icon.width >> icon.height >> icon.data;
    arg.endStructure();
    return arg;
}

// "(sa(iiay)ss)": icon name, icon pixmaps, title, description.
struct StatusNotifierToolTip {
    QString iconName;
    QList<StatusNotifierIconPixmap> iconPixmap;
    QString title;
    QString description;
};

const QDBusArgument &operator>>(const QDBusArgument &arg, StatusNotifierToolTip &tooltip) {
    arg.beginStructure();
    arg >> tooltip.iconName >> tooltip.iconPixmap >> tooltip.title >> tooltip.description;
    arg.endStructure();
    return arg;
}

// Complex GetAll values arrive as a QVariant-wrapped QDBusArgument.
template <typename T>
T demarshall(const QVariant &variant, QDBusArgument::ElementType expected) {
    T result;
    if (!variant.canConvert<QDBusArgument>())
        return result;
    const QDBusArgument arg = variant.value<QDBusArgument>();
    if (arg.currentType() == expected)
        arg >> result;
    return result;
}

QImage decodeIconPixmap(const StatusNotifierIconPixmap &pixmap) {
    if (pixmap.width <= 0 || pixmap.height <= 0
            || pixmap.data.size() < qsizetype(pixmap.width) * pixmap.height * 4)
        return QImage();

    QImage image(pixmap.width, pixmap.height, QImage::Format_ARGB32);
    const uchar *src = reinterpret_cast<const uchar *>(pixmap.data.constData());
    for (int y = 0; y < pixmap.height; ++y) {
        QRgb *destLine = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < pixmap.width; ++x)
            destLine[x] = qFromBigEndian<quint32>(src + (qsizetype(y) * pixmap.width + x) * 4);
    }
    return image;
}

QIcon iconFromPixmaps(const QList<StatusNotifierIconPixmap> &pixmaps) {
    QIcon icon;
    for (const auto &pixmap : pixmaps) {
        const QImage image = decodeIconPixmap(pixmap);
        if (!image.isNull())
            icon.addPixmap(QPixmap::fromImage(image));
    }
    return icon;
}

// IconThemePath is the item's private icon dir; search it directly rather
// than adding it to the app-wide theme search paths.
QIcon iconFromName(const QString &name, const QString &themePath) {
    if (name.isEmpty())
        return QIcon();
    if (name.startsWith('/'))
        return QIcon(name);
    if (QIcon::hasThemeIcon(name))
        return QIcon::fromTheme(name);
    if (themePath.isEmpty())
        return QIcon();

    QIcon icon;
    QDirIterator it(themePath, {name + ".svg", name + ".png", name + ".xpm"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        icon.addFile(it.next());
    return icon;
}

} // namespace

trayicon::trayicon(const QString &service, const QString &path)
    : panelbutton(Icon), m_service(service), m_path(path)
{
    // Hidden until the first GetAll reply, and while Status is Passive.
    setVisible(false);

    connect(this, &panelbutton::leftclicked, this, &trayicon::onLeftClicked);
    connect(this, &panelbutton::rightclicked, this, &trayicon::onRightClicked);
    connect(this, &panelbutton::mouseReleased, this, &trayicon::onMouseReleased);

    QDBusConnection bus = QDBusConnection::sessionBus();
    for (const char *signal : {"NewIcon", "NewAttentionIcon", "NewStatus", "NewToolTip", "NewTitle", "NewIconThemePath", "NewMenu"})
        bus.connect(service, path, kItemInterface, signal, this, SLOT(refresh()));

    refresh();
}

trayicon::~trayicon()
{
}

QPoint trayicon::activationPos() const
{
    return mapToGlobal(QPoint(0, 0));
}

void trayicon::refresh()
{
    // Coalesce signal bursts into at most one in-flight GetAll plus one follow-up.
    if (m_refreshInFlight) {
        m_refreshAgain = true;
        return;
    }
    m_refreshInFlight = true;

    QDBusMessage msg = QDBusMessage::createMethodCall(m_service, m_path,
                                                      QStringLiteral("org.freedesktop.DBus.Properties"),
                                                      QStringLiteral("GetAll"));
    msg << QString::fromLatin1(kItemInterface);
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        m_refreshInFlight = false;

        const QDBusPendingReply<QVariantMap> reply = *watcher;
        if (reply.isError())
            qWarning() << "systray: GetAll failed for" << m_service << reply.error().message();
        else
            applyProperties(reply.value());

        if (m_refreshAgain) {
            m_refreshAgain = false;
            refresh();
        }
    });
}

void trayicon::applyProperties(const QVariantMap &props)
{
    const QString status = props.value("Status").toString();
    const bool needsAttention = (status == QLatin1String("NeedsAttention"));
    const QString themePath = props.value("IconThemePath").toString();

    QIcon icon;
    if (needsAttention) {
        icon = iconFromName(props.value("AttentionIconName").toString(), themePath);
        if (icon.isNull())
            icon = iconFromPixmaps(demarshall<QList<StatusNotifierIconPixmap>>(props.value("AttentionIconPixmap"), QDBusArgument::ArrayType));
    }
    if (icon.isNull())
        icon = iconFromName(props.value("IconName").toString(), themePath);
    if (icon.isNull())
        icon = iconFromPixmaps(demarshall<QList<StatusNotifierIconPixmap>>(props.value("IconPixmap"), QDBusArgument::ArrayType));
    if (icon.isNull())
        icon = QIcon::fromTheme("image-missing");
    setIcon(icon);

    const auto tooltip = demarshall<StatusNotifierToolTip>(props.value("ToolTip"), QDBusArgument::StructureType);
    QString text = tooltip.title;
    if (!tooltip.description.isEmpty())
        text += (text.isEmpty() ? QString() : QStringLiteral("\n")) + tooltip.description;
    if (text.isEmpty())
        text = props.value("Title").toString();
    setToolTip(text);

    m_menuPath = props.value("Menu").value<QDBusObjectPath>().path();

    setVisible(status != QLatin1String("Passive"));
}

void trayicon::callItem(const QString &method, const QVariantList &args)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(m_service, m_path, kItemInterface, method);
    msg.setArguments(args);
    QDBusConnection::sessionBus().asyncCall(msg);
}

void trayicon::onLeftClicked()
{
    const QPoint pos = activationPos();
    callItem("Activate", {pos.x(), pos.y()});
}

void trayicon::onRightClicked()
{
    if (!m_menuPath.isEmpty() && m_menuPath != QLatin1String("/")) {
        if (menuImporter) {
            menuImporter->updateMenu();
            return;
        }
        // DBusMenuImporter's ctor introspects synchronously, so only create it
        // once the app has answered an async Introspect.
        if (m_menuProbeInFlight)
            return;
        m_menuProbeInFlight = true;
        QDBusMessage msg = QDBusMessage::createMethodCall(m_service, m_menuPath,
                                                          QStringLiteral("org.freedesktop.DBus.Introspectable"),
                                                          QStringLiteral("Introspect"));
        auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();
            m_menuProbeInFlight = false;
            if (watcher->isError() || menuImporter)
                return;
            menuImporter = new DBusMenuImporter(m_service, m_menuPath, this);
            // The layout is fetched async; only popup() once it's populated.
            connect(menuImporter, &DBusMenuImporter::menuUpdated, this, &trayicon::showTrayMenu);
            menuImporter->updateMenu();
        });
        return;
    }

    const QPoint pos = activationPos();
    callItem("ContextMenu", {pos.x(), pos.y()});
}

void trayicon::showTrayMenu()
{
    QMenu *menu = menuImporter->menu();
    anchorMenuOnLauncher(menu, this, CenteredOnWidget);
    menu->popup(mapToGlobal(QPoint(0, 0)));
}

void trayicon::onMouseReleased(QMouseEvent *event)
{
    if (event->button() != Qt::MiddleButton)
        return;

    const QPoint pos = activationPos();
    callItem("SecondaryActivate", {pos.x(), pos.y()});
}

void trayicon::wheelEvent(QWheelEvent *event)
{
    const QPoint delta = event->angleDelta();
    if (delta.y() != 0)
        callItem("Scroll", {delta.y() / 120, QStringLiteral("vertical")});
    if (delta.x() != 0)
        callItem("Scroll", {delta.x() / 120, QStringLiteral("horizontal")});

    event->accept();
}
