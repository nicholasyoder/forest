// SPDX-License-Identifier: LGPL-3.0-or-later

#include "trayicon.h"

#include <QCursor>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QImage>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QtEndian>
#include <QWheelEvent>

#include <dbusmenuimporter.h>

// One entry of IconPixmap/AttentionIconPixmap ("a(iiay)"): width, height,
// and raw pixel data in ARGB32 *network byte order* (big-endian bytes per
// pixel) - not Qt's native-endian QImage::Format_ARGB32, so decoding below
// has to byte-swap each pixel rather than memcpy the buffer directly.
//
// All the operator<</>> overloads and Q_DECLARE_METATYPE calls below are
// deliberately at plain file scope, not wrapped in an anonymous namespace -
// mirrors Qt's own StatusNotifierItem publisher (qtbase's
// qdbustraytypes.cpp/.h) exactly, after wrapping these in a namespace
// turned out to break ADL: StatusNotifierToolTip's own operator>> streams
// a QList<StatusNotifierIconPixmap> field, and needs to find this file's
// custom operator>> for that exact list type via ordinary lookup - putting
// them in different (even if both anonymous) namespace blocks was enough
// to make that fail and silently fall back to QDBusArgument's generic,
// less reliable QList<T> template instead.
struct StatusNotifierIconPixmap {
    int width = 0;
    int height = 0;
    QByteArray data;
};

Q_DECLARE_METATYPE(StatusNotifierIconPixmap)

// operator<< (serialize) is never used to actually send this type over
// DBus, but qDBusRegisterMetaType() needs it too - it streams a
// default-constructed instance through a temporary QDBusArgument to derive
// the type's D-Bus signature at registration time.
QDBusArgument &operator<<(QDBusArgument &arg, const StatusNotifierIconPixmap &icon) {
    arg.beginStructure();
    arg << icon.width << icon.height << icon.data;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, StatusNotifierIconPixmap &icon) {
    arg.beginStructure();
    arg >> icon.width >> icon.height >> icon.data;
    arg.endStructure();
    return arg;
}

Q_DECLARE_METATYPE(QList<StatusNotifierIconPixmap>)

// Explicit (non-template) overloads for the list, rather than relying on
// QDBusArgument's generic QList<T> template - confirmed against Qt's own
// StatusNotifierItem publisher that this is required, not just style:
// qDBusRegisterMetaType() derives a type's D-Bus signature by streaming a
// *default-constructed* (so, empty) instance through a throwaway
// QDBusArgument. For an empty list the loop body never runs, so the
// generic template - which only learns the element type from what it
// happens to stream - never communicates an element type to beginArray()
// at all, and the computed signature comes out wrong. Passing the element
// type explicitly via beginArray(qMetaTypeId<...>()) fixes it regardless
// of whether the list is empty.
QDBusArgument &operator<<(QDBusArgument &arg, const QList<StatusNotifierIconPixmap> &icons) {
    arg.beginArray(qMetaTypeId<StatusNotifierIconPixmap>());
    for (const auto &icon : icons)
        arg << icon;
    arg.endArray();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, QList<StatusNotifierIconPixmap> &icons) {
    icons.clear();
    arg.beginArray();
    while (!arg.atEnd()) {
        StatusNotifierIconPixmap icon;
        arg >> icon;
        icons.append(icon);
    }
    arg.endArray();
    return arg;
}

// ToolTip property: "(s a(iiay) s s)" - icon name, icon pixmap(s), title,
// description. Only the text fields are surfaced as a plain Qt tooltip for
// now, not the tooltip's own icon.
struct StatusNotifierToolTip {
    QString iconName;
    QList<StatusNotifierIconPixmap> iconPixmap;
    QString title;
    QString description;
};

Q_DECLARE_METATYPE(StatusNotifierToolTip)

QDBusArgument &operator<<(QDBusArgument &arg, const StatusNotifierToolTip &tooltip) {
    arg.beginStructure();
    arg << tooltip.iconName << tooltip.iconPixmap << tooltip.title << tooltip.description;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, StatusNotifierToolTip &tooltip) {
    arg.beginStructure();
    arg >> tooltip.iconName >> tooltip.iconPixmap >> tooltip.title >> tooltip.description;
    arg.endStructure();
    return arg;
}

namespace {

constexpr char kItemInterface[] = "org.kde.StatusNotifierItem";

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

// Property arrives as a size-sorted-by-nothing list of the same icon at
// different resolutions - take the largest for the best downscale quality.
QImage largestIcon(const QList<StatusNotifierIconPixmap> &pixmaps) {
    const StatusNotifierIconPixmap *best = nullptr;
    for (const auto &pixmap : pixmaps) {
        if (!best || pixmap.width > best->width)
            best = &pixmap;
    }
    return best ? decodeIconPixmap(*best) : QImage();
}

// QDBusInterface::property(name) resolves a property's C++ type by
// introspecting the target's XML, but its reverse signature->type lookup
// (QDBusMetaType::signatureToMetaType(), qtbase/src/dbus/qdbusmetatype.cpp)
// only ever recognizes *basic* D-Bus types (bool/int/string/...) - it never
// consults the qDBusRegisterMetaType() registry at all. A registered
// complex type (like these) is only found by that introspection if the
// server's own XML carries a "org.qtproject.QtDBus.QtTypeName" annotation
// naming the exact registered type - a Qt-to-Qt-only convenience that
// Qt's own qdbusxml2cpp-generated adaptors emit automatically (which is
// why a Qt host reading a Qt app's tray icon just works), but that a
// non-Qt-generated StatusNotifierItem implementation (e.g. a Python app)
// has no reason to include. Without it, property() fails outright for
// these three - registering the types client-side, however correctly,
// can't fix a check that never looks at the registry to begin with.
//
// So these three are read via a raw org.freedesktop.DBus.Properties.Get
// call (getRawProperty(), below) instead of item->property(name) - that
// path skips the metaobject-property gate entirely and hands back the
// value as get() actually receives it off the wire: a QVariant wrapping a
// QDBusArgument, which readIconPixmapList()/readToolTip() decode by hand
// via the operator>> overloads above.
QVariant getRawProperty(QDBusInterface *item, const char *interfaceName, const QString &propertyName) {
    QDBusMessage msg = QDBusMessage::createMethodCall(item->service(), item->path(),
                                                       QStringLiteral("org.freedesktop.DBus.Properties"),
                                                       QStringLiteral("Get"));
    msg << QString::fromLatin1(interfaceName) << propertyName;
    const QDBusMessage reply = item->connection().call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
        return QVariant();
    return qvariant_cast<QDBusVariant>(reply.arguments().at(0)).variant();
}

QList<StatusNotifierIconPixmap> readIconPixmapList(const QVariant &variant) {
    if (variant.canConvert<QList<StatusNotifierIconPixmap>>())
        return variant.value<QList<StatusNotifierIconPixmap>>();

    QList<StatusNotifierIconPixmap> result;
    QDBusArgument arg = variant.value<QDBusArgument>();
    if (arg.currentType() == QDBusArgument::ArrayType)
        arg >> result;
    return result;
}

StatusNotifierToolTip readToolTip(const QVariant &variant) {
    if (variant.canConvert<StatusNotifierToolTip>())
        return variant.value<StatusNotifierToolTip>();

    StatusNotifierToolTip tooltip;
    QDBusArgument arg = variant.value<QDBusArgument>();
    if (arg.currentType() == QDBusArgument::StructureType)
        arg >> tooltip;
    return tooltip;
}

// Kept even though getRawProperty() no longer depends on it: harmless, and
// covers the (rare, Qt-to-Qt) case where a publisher's XML does carry the
// QtTypeName annotation, in which case property()/canConvert() above would
// resolve directly to these types without needing the raw fallback at all.
struct MetaTypeRegistrar {
    MetaTypeRegistrar() {
        qDBusRegisterMetaType<StatusNotifierIconPixmap>();
        qDBusRegisterMetaType<QList<StatusNotifierIconPixmap>>();
        qDBusRegisterMetaType<StatusNotifierToolTip>();
    }
};
const MetaTypeRegistrar metaTypeRegistrar;

} // namespace

trayicon::trayicon(const QString &service, const QString &path)
    : m_service(service), m_path(path)
{
    // Deliberately not setObjectName()'d - keep panelbutton's inherited
    // "panelButton", the same as every other panel plugin's buttons, so
    // this picks up the existing #panelButton[buttontype="Icon"] QSS rule
    // (min-width/icon-size) instead of matching no selector at all.
    item = new QDBusInterface(service, path, kItemInterface, QDBusConnection::sessionBus(), this);

    connect(this, &panelbutton::leftclicked, this, &trayicon::onLeftClicked);
    connect(this, &panelbutton::rightclicked, this, &trayicon::onRightClicked);
    connect(this, &panelbutton::mouseReleased, this, &trayicon::onMouseReleased);

    QDBusConnection::sessionBus().connect(service, path, kItemInterface, "NewIcon", this, SLOT(onNewIcon()));
    QDBusConnection::sessionBus().connect(service, path, kItemInterface, "NewAttentionIcon", this, SLOT(onNewAttentionIcon()));
    QDBusConnection::sessionBus().connect(service, path, kItemInterface, "NewStatus", this, SLOT(onNewStatus(QString)));
    QDBusConnection::sessionBus().connect(service, path, kItemInterface, "NewToolTip", this, SLOT(onNewToolTip()));
    QDBusConnection::sessionBus().connect(service, path, kItemInterface, "NewTitle", this, SLOT(onNewTitle()));

    m_status = item->property("Status").toString();
    updateIcon();
    updateToolTip();
}

trayicon::~trayicon()
{
}

QPoint trayicon::activationPos() const
{
    return mapToGlobal(QPoint(0, 0));
}

void trayicon::updateIcon()
{
    const bool needsAttention = (m_status == QLatin1String("NeedsAttention"));
    const QString themePath = item->property("IconThemePath").toString();

    if (!themePath.isEmpty() && !QIcon::themeSearchPaths().contains(themePath))
        QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << themePath);

    const QString iconName = item->property(needsAttention ? "AttentionIconName" : "IconName").toString();
    if (!iconName.isEmpty()) {
        setupIconButton(iconName);
        return;
    }

    const QVariant pixmapVariant = getRawProperty(item, kItemInterface, needsAttention ? QStringLiteral("AttentionIconPixmap") : QStringLiteral("IconPixmap"));
    const auto pixmaps = readIconPixmapList(pixmapVariant);
    const QImage image = largestIcon(pixmaps);
    if (!image.isNull())
        setupIconButton(QIcon(QPixmap::fromImage(image)));
    else
        setupIconButton(QIcon::fromTheme("image-missing"));
}

void trayicon::updateToolTip()
{
    const auto tooltip = readToolTip(getRawProperty(item, kItemInterface, QStringLiteral("ToolTip")));

    QString text = tooltip.title;
    if (!tooltip.description.isEmpty())
        text += (text.isEmpty() ? QString() : QStringLiteral("\n")) + tooltip.description;
    if (text.isEmpty())
        text = item->property("Title").toString();

    setToolTip(text);
}

void trayicon::onLeftClicked()
{
    const QPoint pos = activationPos();
    item->asyncCall("Activate", pos.x(), pos.y());
}

void trayicon::onRightClicked()
{
    const QString menuPath = item->property("Menu").value<QDBusObjectPath>().path();
    if (!menuPath.isEmpty() && menuPath != QLatin1String("/")) {
        if (!menuImporter)
            menuImporter = new DBusMenuImporter(m_service, menuPath, this);
        menuImporter->updateMenu();
        menuImporter->menu()->popup(QCursor::pos());
        return;
    }

    const QPoint pos = activationPos();
    item->asyncCall("ContextMenu", pos.x(), pos.y());
}

void trayicon::onMouseReleased(QMouseEvent *event)
{
    if (event->button() != Qt::MiddleButton)
        return;

    const QPoint pos = activationPos();
    item->asyncCall("SecondaryActivate", pos.x(), pos.y());
}

void trayicon::wheelEvent(QWheelEvent *event)
{
    const QPoint delta = event->angleDelta();
    if (delta.y() != 0)
        item->asyncCall("Scroll", delta.y() / 120, QStringLiteral("vertical"));
    if (delta.x() != 0)
        item->asyncCall("Scroll", delta.x() / 120, QStringLiteral("horizontal"));

    event->accept();
}

void trayicon::onNewIcon() { updateIcon(); }
void trayicon::onNewAttentionIcon() { updateIcon(); }
void trayicon::onNewToolTip() { updateToolTip(); }
void trayicon::onNewTitle() { updateToolTip(); }

void trayicon::onNewStatus(const QString &status)
{
    m_status = status;
    updateIcon();
}
