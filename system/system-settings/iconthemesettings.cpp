// SPDX-License-Identifier: LGPL-3.0-or-later

#include "iconthemesettings.h"
#include "wideicondelegate.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QPainter>
#include <QQueue>
#include <QSet>
#include <QSettings>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QTextStream>
#include <algorithm>

#include <forest-theme/foresttheme.h>
#include "../../library/fstyleloader/toolkitsync.h"

namespace {

constexpr int kPreviewIconSize = 24;
constexpr int kPreviewSpacing = 6;
constexpr int kPreviewPadding = 8;

const QList<QStringList> kPreviewSlots = {
    {QStringLiteral("folder"), QStringLiteral("inode-directory")},
    {QStringLiteral("user-home"), QStringLiteral("go-home")},
    {QStringLiteral("text-x-generic"), QStringLiteral("document-new")},
    {QStringLiteral("preferences-system"), QStringLiteral("applications-system")},
    {QStringLiteral("utilities-terminal"), QStringLiteral("system-run"), QStringLiteral("application-x-executable")},
    {QStringLiteral("audio-volume-high"), QStringLiteral("multimedia-volume-control"), QStringLiteral("image-x-generic")},
};

struct SubdirInfo {
    QString path;
    int size = 0;
    int minSize = 0;
    int maxSize = 0;
    bool scalable = false;
    bool symbolic = false;
};

struct ThemeMeta {
    QString id;
    QString name;
    bool hidden = false;
    bool hasDirectories = false;
    QStringList inherits;
    QStringList baseDirs;
    QList<SubdirInfo> subdirs;
};

QStringList iconSearchRoots()
{
    QStringList dirs = {QDir::home().filePath(QStringLiteral(".icons"))};
    for (const QString &path : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        dirs.append(QDir(path).filePath(QStringLiteral("icons")));
    dirs.removeDuplicates();
    return dirs;
}

int subdirScore(const SubdirInfo &d)
{
    int score = 0;
    if (d.symbolic)
        score += 10000;
    if (d.scalable) {
        if (d.minSize <= kPreviewIconSize && kPreviewIconSize <= d.maxSize)
            return score + 10;
        return score + 50 + qAbs(d.size - kPreviewIconSize);
    }
    if (d.size == kPreviewIconSize)
        return score;
    if (d.size > kPreviewIconSize)
        return score + 20 + (d.size - kPreviewIconSize);
    return score + 200 + (kPreviewIconSize - d.size);
}

// Parses index.theme line-by-line so commas in Name/Comment aren't split by QSettings.
void parseIndexTheme(const QString &indexPath, ThemeMeta &meta)
{
    QFile f(indexPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString section;
    QHash<QString, SubdirInfo> subdirMap;
    QStringList dirList;
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;
        if (line.startsWith('[') && line.endsWith(']')) {
            section = line.mid(1, line.size() - 2);
            continue;
        }
        const int eq = line.indexOf('=');
        if (eq <= 0)
            continue;
        const QString key = line.left(eq).trimmed();
        const QString val = line.mid(eq + 1).trimmed();

        if (section == QLatin1String("Icon Theme")) {
            if (key == QLatin1String("Name") && meta.name.isEmpty())
                meta.name = val;
            else if (key == QLatin1String("Hidden"))
                meta.hidden = (val.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
            else if (key == QLatin1String("Inherits") && meta.inherits.isEmpty())
                meta.inherits = val.split(',', Qt::SkipEmptyParts);
            else if (key == QLatin1String("Directories") && dirList.isEmpty())
                dirList = val.split(',', Qt::SkipEmptyParts);
        } else if (!section.isEmpty()) {
            SubdirInfo &sd = subdirMap[section];
            sd.path = section;
            if (key == QLatin1String("Size"))
                sd.size = val.toInt();
            else if (key == QLatin1String("MinSize"))
                sd.minSize = val.toInt();
            else if (key == QLatin1String("MaxSize"))
                sd.maxSize = val.toInt();
            else if (key == QLatin1String("Type"))
                sd.scalable = (val.compare(QLatin1String("Scalable"), Qt::CaseInsensitive) == 0);
            else if (key == QLatin1String("Scale") && val.toInt() > 1)
                sd.size *= val.toInt();
        }
    }

    for (QString &inh : meta.inherits)
        inh = inh.trimmed();
    if (!dirList.isEmpty())
        meta.hasDirectories = true;
    for (const QString &rawDir : dirList) {
        const QString dir = rawDir.trimmed();
        if (dir.isEmpty())
            continue;
        SubdirInfo sd = subdirMap.value(dir);
        sd.path = dir;
        if (sd.size <= 0)
            sd.size = 32;
        if (sd.minSize <= 0)
            sd.minSize = sd.size;
        if (sd.maxSize <= 0)
            sd.maxSize = sd.size;
        sd.symbolic = dir.contains(QLatin1String("symbolic"), Qt::CaseInsensitive);
        meta.subdirs.append(sd);
    }
    std::stable_sort(meta.subdirs.begin(), meta.subdirs.end(), [](const SubdirInfo &a, const SubdirInfo &b) {
        return subdirScore(a) < subdirScore(b);
    });
}

QString findIconFileInTheme(const ThemeMeta &meta, const QStringList &candidates)
{
    static const QStringList exts = {QStringLiteral(".svg"), QStringLiteral(".png"), QStringLiteral(".xpm")};
    for (const SubdirInfo &sd : meta.subdirs) {
        for (const QString &baseDir : meta.baseDirs) {
            const QString subPath = baseDir + QLatin1Char('/') + sd.path;
            if (!QFileInfo::exists(subPath))
                continue;
            for (const QString &name : candidates) {
                for (const QString &ext : exts) {
                    const QString file = subPath + QLatin1Char('/') + name + ext;
                    if (QFileInfo::exists(file))
                        return file;
                }
            }
        }
    }
    return {};
}

QString resolveIconFile(const QString &themeId, const QHash<QString, ThemeMeta> &allThemes,
                        const QStringList &candidates)
{
    QQueue<QString> queue;
    QSet<QString> visited;
    queue.enqueue(themeId);
    visited.insert(themeId);

    while (!queue.isEmpty()) {
        const QString id = queue.dequeue();
        auto it = allThemes.constFind(id);
        if (it == allThemes.constEnd())
            continue;
        const QString found = findIconFileInTheme(*it, candidates);
        if (!found.isEmpty())
            return found;
        for (const QString &parent : it->inherits) {
            if (!visited.contains(parent)) {
                visited.insert(parent);
                queue.enqueue(parent);
            }
        }
    }
    if (!visited.contains(QStringLiteral("hicolor"))) {
        auto it = allThemes.constFind(QStringLiteral("hicolor"));
        if (it != allThemes.constEnd())
            return findIconFileInTheme(*it, candidates);
    }
    return {};
}

QIcon buildPreviewStrip(const QString &themeId, const QHash<QString, ThemeMeta> &allThemes, qreal dpr)
{
    const int count = kPreviewSlots.size();
    const int w = kPreviewPadding * 2 + count * kPreviewIconSize + (count - 1) * kPreviewSpacing;
    const int h = kPreviewIconSize;

    QPixmap strip(qRound(w * dpr), qRound(h * dpr));
    strip.setDevicePixelRatio(dpr);
    strip.fill(Qt::transparent);

    QPainter painter(&strip);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    int x = kPreviewPadding;
    for (const QStringList &slot : kPreviewSlots) {
        const QString file = resolveIconFile(themeId, allThemes, slot);
        if (!file.isEmpty()) {
            const QPixmap pm = QIcon(file).pixmap(QSize(kPreviewIconSize, kPreviewIconSize), dpr);
            if (!pm.isNull())
                painter.drawPixmap(QRect(x, 0, kPreviewIconSize, kPreviewIconSize), pm);
        }
        x += kPreviewIconSize + kPreviewSpacing;
    }
    return QIcon(strip);
}

} // namespace

IconThemeSettings::IconThemeSettings()
{
    settings_item = new settings_page("appearance/icons", "Icons", "preferences-desktop-icons", 5);
    settings_item->set_keywords({"icon", "icons", "theme", "papirus", "adwaita", "breeze"});
    connect(settings_item, &settings_category::opened, this, &IconThemeSettings::load_icon_themes);

    icon_theme_list = new ListWidget();
    icon_theme_list->setMinimumHeight(5);
    icon_theme_list->setExclusiveCheck(true);
    icon_theme_list->setSelectionMode(QAbstractItemView::NoSelection);
    const int count = kPreviewSlots.size();
    const int previewW = kPreviewPadding * 2 + count * kPreviewIconSize + (count - 1) * kPreviewSpacing;
    icon_theme_list->setItemDelegate(new WideIconDelegate(icon_theme_list, QSize(previewW, kPreviewIconSize)));
    settings_item->add_child(new settings_widget("", "", icon_theme_list));

    connect(icon_theme_list, &ListWidget::itemExclusivlySelected, this, &IconThemeSettings::set_icon_theme);
}

void IconThemeSettings::load_icon_themes()
{
    QHash<QString, ThemeMeta> allThemes;
    for (const QString &root : iconSearchRoots()) {
        QDir dir(root);
        for (const QString &entry : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString themeDir = dir.filePath(entry);
            const QString indexPath = themeDir + QStringLiteral("/index.theme");
            if (!QFileInfo::exists(indexPath))
                continue;
            ThemeMeta &meta = allThemes[entry];
            const bool firstSeen = meta.id.isEmpty();
            meta.id = entry;
            meta.baseDirs.append(themeDir);
            if (firstSeen)
                parseIndexTheme(indexPath, meta);
        }
    }

    QList<ThemeMeta> visible;
    for (auto it = allThemes.constBegin(); it != allThemes.constEnd(); ++it) {
        if (!it->hidden && it->hasDirectories)
            visible.append(*it);
    }
    std::sort(visible.begin(), visible.end(), [](const ThemeMeta &a, const ThemeMeta &b) {
        const QString na = a.name.isEmpty() ? a.id : a.name;
        const QString nb = b.name.isEmpty() ? b.id : b.name;
        return na.localeAwareCompare(nb) < 0;
    });

    QSettings settings(QStringLiteral("Forest"), QStringLiteral("Forest"));
    const QString current = settings.value(QStringLiteral("icon_theme"), foresttheme::defaultIconTheme()).toString();

    const QSignalBlocker blocker(icon_theme_list);
    icon_theme_list->clear();
    const qreal dpr = icon_theme_list->devicePixelRatioF();

    for (const ThemeMeta &theme : visible) {
        auto *item = new QListWidgetItem(theme.name.isEmpty() ? theme.id : theme.name);
        item->setData(Qt::UserRole, theme.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(theme.id == current ? Qt::Checked : Qt::Unchecked);
        item->setIcon(buildPreviewStrip(theme.id, allThemes, dpr));
        icon_theme_list->addItem(item);
    }
}

void IconThemeSettings::set_icon_theme(QListWidgetItem *item)
{
    const QString newTheme = item->data(Qt::UserRole).toString();
    QSettings settings(QStringLiteral("Forest"), QStringLiteral("Forest"));
    settings.setValue(QStringLiteral("icon_theme"), newTheme);
    settings.sync();

    toolkitsync::applyIconTheme(newTheme);
}
