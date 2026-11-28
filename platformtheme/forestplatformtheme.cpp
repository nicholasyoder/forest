// SPDX-License-Identifier: LGPL-3.0-or-later

#include "forestplatformtheme.h"

#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPixmapCache>
#include <QSettings>
#include <QWindow>
#include <qpa/qwindowsysteminterface.h>

#include <private/xdgiconloader/xdgiconloader_p.h>
#include <forest-theme/foresttheme.h>

namespace {

QPalette buildPalette(const QString &dir, const QStringList &layers, Qt::ColorScheme scheme, bool *found)
{
    QPalette base;
    // Seed unconfigured roles (BrightText, Inactive, Disabled) from dark/light defaults before overlaying theme.conf.
    if (scheme == Qt::ColorScheme::Dark) {
        base = QPalette(QColor(0x32, 0x32, 0x32), QColor(0x1e, 0x1e, 0x1e));
        base.setColor(QPalette::BrightText, Qt::white);
    } else {
        base = QPalette(QColor(0xf5, 0xf5, 0xf5), QColor(0xeb, 0xeb, 0xeb));
        base.setColor(QPalette::BrightText, Qt::black);
    }
    QPalette pal = foresttheme::palette(dir, layers, base, found);
    if (*found) {
        const QColor link = pal.color(QPalette::Link);
        pal.setColor(QPalette::All, QPalette::LinkVisited,
                     scheme == Qt::ColorScheme::Dark ? link.lighter(115) : link.darker(120));
        const QColor hi = pal.color(QPalette::Highlight);
        pal.setColor(QPalette::Disabled, QPalette::Highlight, pal.color(QPalette::Mid));
        pal.setColor(QPalette::Disabled, QPalette::HighlightedText, pal.color(QPalette::Disabled, QPalette::Text));
        pal.setColor(QPalette::Inactive, QPalette::Highlight,
                     scheme == Qt::ColorScheme::Dark ? hi.darker(135) : hi.lighter(125));
    }
    return pal;
}

} // namespace

ForestPlatformTheme::ForestPlatformTheme()
    : QGenericUnixTheme()
    , m_helper(new QObject)
    , m_systemFont(QStringLiteral("DejaVu Sans"), 10)
    , m_fixedFont(QStringLiteral("JetBrains Mono"), 10)
{
    m_fixedFont.setStyleHint(QFont::TypeWriter);
    reloadConfig();

    // Defer watcher setup until QGuiApplication finishes constructing its event dispatcher.
    QTimer::singleShot(0, m_helper, [this]() {
        startWatcher();
    });
}

ForestPlatformTheme::~ForestPlatformTheme()
{
    delete m_helper;
}

QString ForestPlatformTheme::configDirPath()
{
    const QString cfg = qEnvironmentVariable("XDG_CONFIG_HOME");
    const QString base = cfg.isEmpty() ? QDir::home().filePath(QStringLiteral(".config")) : cfg;
    return QDir(base).filePath(QStringLiteral("Forest"));
}

QString ForestPlatformTheme::configFilePath()
{
    return QDir(configDirPath()).filePath(QStringLiteral("Forest.conf"));
}

bool ForestPlatformTheme::reloadConfig()
{
    QSettings conf(configFilePath(), QSettings::IniFormat);
    const QString themeName = conf.value(QStringLiteral("theme"), foresttheme::defaultTheme()).toString();
    const QString iconTheme = conf.value(QStringLiteral("icon_theme"), foresttheme::defaultIconTheme()).toString();

    const QString dir = foresttheme::defaultDir();
    const QStringList layers = foresttheme::chain(dir, themeName);
    const Qt::ColorScheme scheme = foresttheme::colorScheme(dir, layers);
    bool hasPalette = false;
    const QPalette pal = buildPalette(dir, layers, scheme, &hasPalette);

    const bool changed = (themeName != m_themeName)
                      || (iconTheme != m_iconTheme)
                      || (scheme != m_colorScheme)
                      || (hasPalette != m_hasPalette)
                      || (hasPalette && pal != m_palette);

    m_themeName = themeName;
    m_iconTheme = iconTheme;
    m_colorScheme = scheme;
    m_hasPalette = hasPalette;
    m_palette = pal;
    return changed;
}

void ForestPlatformTheme::startWatcher()
{
    if (m_watcher)
        return;
    m_watcher = new QFileSystemWatcher(m_helper);
    m_debounce = new QTimer(m_helper);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(50);

    watchConfigPaths();

    auto schedule = [this]() {
        watchConfigPaths();
        m_debounce->start();
    };
    QObject::connect(m_watcher, &QFileSystemWatcher::fileChanged, m_helper, schedule);
    QObject::connect(m_watcher, &QFileSystemWatcher::directoryChanged, m_helper, schedule);
    QObject::connect(m_debounce, &QTimer::timeout, m_helper, [this]() { onConfigChanged(); });
}

void ForestPlatformTheme::watchConfigPaths()
{
    // Watch both the directory and the file: QSettings saves atomically via rename.
    const QString dir = configDirPath();
    const QString file = configFilePath();
    if (QFileInfo::exists(dir) && !m_watcher->directories().contains(dir))
        m_watcher->addPath(dir);
    if (QFileInfo::exists(file) && !m_watcher->files().contains(file))
        m_watcher->addPath(file);
}

void ForestPlatformTheme::onConfigChanged()
{
    if (!reloadConfig())
        return;

    QWindowSystemInterface::handleThemeChange();

    // Repaint windows after Qt's queued processThemeChanged() updates the icon loader and palette.
    QTimer::singleShot(0, m_helper, []() {
        QPixmapCache::clear();
        for (QWindow *w : QGuiApplication::topLevelWindows()) {
            QEvent ev(QEvent::ThemeChange);
            QCoreApplication::sendEvent(w, &ev);
            w->requestUpdate();
        }
    });
}

const QPalette *ForestPlatformTheme::palette(Palette type) const
{
    if (m_hasPalette && type == SystemPalette)
        return &m_palette;
    return QGenericUnixTheme::palette(type);
}

const QFont *ForestPlatformTheme::font(Font type) const
{
    switch (type) {
    case SystemFont:
        return &m_systemFont;
    case FixedFont:
        return &m_fixedFont;
    default:
        return QGenericUnixTheme::font(type);
    }
}

QVariant ForestPlatformTheme::themeHint(ThemeHint hint) const
{
    switch (hint) {
    case SystemIconThemeName:
        return m_iconTheme;
    case SystemIconFallbackThemeName:
        return QStringLiteral("hicolor");
    case ShowIconsInMenus:
        return true;
    case ShowShortcutsInContextMenus:
        return true;
    case DialogButtonBoxButtonsHaveIcons:
        return true;
    case ToolButtonStyle:
        return int(Qt::ToolButtonFollowStyle);
    case UnderlineShortcut:
        return true;
    default:
        return QGenericUnixTheme::themeHint(hint);
    }
}

Qt::ColorScheme ForestPlatformTheme::colorScheme() const
{
    return m_colorScheme;
}

QIcon ForestPlatformTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions) const
{
    return xdgFileIcon(fileInfo);
}

// XdgIconLoaderEngine recolors FollowsColorScheme=true SVGs (e.g. Papirus panel icons)
// from QGuiApplication::palette(), which Qt's built-in QIconLoaderEngine does not.
QIconEngine *ForestPlatformTheme::createIconEngine(const QString &iconName) const
{
    return new XdgIconLoaderEngine(iconName);
}
