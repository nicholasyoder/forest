// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cursorthemesettings.h"

#include <QDir>
#include <QLabel>
#include <QProcess>
#include <QSettings>

#include <X11/Xcursor/Xcursor.h>

namespace {
constexpr int kDefaultCursorSize = 24;
}

QString cursor_preview_names[] = {
    "left_ptr",
    "left_ptr_watch",
    "wait",
    "hand2",
    "text",
    "size_all",
    "size_fdiag",
    "cross"
};

CursorThemeSettings::CursorThemeSettings(){
    settings_item = new settings_page("appearance/cursor", "Cursor", "preferences-desktop-mouse", 20);
    settings_item->set_keywords({"mouse", "pointer", "size"});
    connect(settings_item, &settings_category::opened, this, &CursorThemeSettings::load_cursor_themes);

    QLabel *note_label = new QLabel(tr(
        "Changes may not fully take effect until you log out and back in."));
    note_label->setObjectName("SettingsNoteLabel");
    note_label->setWordWrap(true);
    settings_item->add_child(new settings_widget("", "", note_label));

    size_input = new QSpinBox;
    size_input->setMaximum(128);
    connect(size_input, &QSpinBox::textChanged, this, &CursorThemeSettings::set_cursor_size);
    settings_widget *test_item = new settings_widget("Size", "", size_input);
    settings_item->add_child(test_item);

    cursor_theme_list = new ListWidget();
    cursor_theme_list->setMinimumHeight(5);
    cursor_theme_list->setExclusiveCheck(true);
    cursor_theme_list->setSelectionMode(QAbstractItemView::NoSelection);
    int cursor_preview_size = 22;
    int cursor_count = sizeof(cursor_preview_names) / sizeof(cursor_preview_names[0]);
    cursor_theme_list->setItemDelegate(new WideIconDelegate(cursor_theme_list, QSize(cursor_count*cursor_preview_size, cursor_preview_size)));
    settings_widget *cursor_theme_list_item = new settings_widget("","", cursor_theme_list);
    settings_item->add_child(cursor_theme_list_item);

    connect(cursor_theme_list, &ListWidget::itemExclusivlySelected, this, &CursorThemeSettings::set_cursor_theme);
}

void CursorThemeSettings::load_cursor_themes(){
    QStringList theme_list;

    // Check some default dirs as well as XDG_DATA_DIRS
    QStringList theme_dirs = {
        "/usr/share/icons",
        "/usr/local/share/icons",
        QDir::home().filePath(".icons"),
        QDir::home().filePath(".local/share/icons")
    };
    QString xdgDataDirs = qgetenv("XDG_DATA_DIRS");
    if (!xdgDataDirs.isEmpty()) {
        foreach (QString path, xdgDataDirs.split(":")) {
            if(!theme_dirs.contains(path))
                theme_dirs.append(path);
        }
    }
    foreach(QString dir_path, theme_dirs) {
        QDir dir(dir_path);
        QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QString theme, themes) {
            QString cursors_path = dir.filePath(theme + "/cursors");
            if (QDir(cursors_path).exists()) {
                theme_list.append(theme);
            }
        }
    }

    theme_list.sort();
    cursor_theme_list->clear();

    // The Xcursor fallback theme file; set_cursor_theme() writes it, so it's also our saved state.
    QSettings index_theme(
        QDir::home().filePath(QStringLiteral(".icons/default/index.theme")), QSettings::IniFormat);
    index_theme.beginGroup(QStringLiteral("Icon Theme"));
    QString current_theme = index_theme.value(QStringLiteral("Inherits")).toString();
    int current_size = index_theme.value(QStringLiteral("Size"), kDefaultCursorSize).toInt();
    index_theme.endGroup();

    foreach (QString theme, theme_list) {
        QListWidgetItem *item = new QListWidgetItem(theme);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(current_theme == theme ? Qt::Checked : Qt::Unchecked);
        QList<QPixmap> pixmaps;
        for(const QString &cursor : cursor_preview_names){
            pixmaps.append(get_cursor_preview(theme, cursor));
        }
        item->setIcon(QIcon(combine_pixmaps(pixmaps)));
        cursor_theme_list->addItem(item);
    }
    size_input->setValue(current_size);
}

QPixmap CursorThemeSettings::get_cursor_preview(QString theme, QString cursor, int size){
    QByteArray theme_name = QFile::encodeName(theme);
    QByteArray cursor_name = QFile::encodeName(cursor);
    XcursorImage *xc_image = XcursorLibraryLoadImage(cursor_name.constData(), theme_name.constData(), size);
    if (!xc_image) {
        return QPixmap();
    }
    QImage qImage((uchar *)xc_image->pixels, xc_image->width, xc_image->height, QImage::Format_ARGB32);
    QPixmap pixmap = QPixmap::fromImage(qImage);
    XcursorImageDestroy(xc_image);
    return pixmap;
}

QPixmap CursorThemeSettings::combine_pixmaps(QList<QPixmap> pixmaps){
    if (pixmaps.isEmpty())
        return QPixmap();

    int padding = 20;
    int new_width = 0;
    int new_height = 0;

    // Calculate the size of the resulting pixmap
    foreach(QPixmap pixmap, pixmaps){
        new_width += pixmap.width();
        new_height = qMax(new_height, pixmap.height());
    }
    new_width += padding*2;

    // Create the resulting pixmap
    QPixmap result(new_width, new_height);
    result.fill(Qt::transparent); // Fill with transparent color

    // Draw the pixmaps onto the result
    QPainter painter(&result);
    int x = padding;
    foreach(QPixmap pixmap, pixmaps){
        painter.drawPixmap(x, 0, pixmap);
        x += pixmap.width();
    }

    return result;
}

// Get currently check item and call set_cursor_theme
// Called on size_input textChanged
void CursorThemeSettings::set_cursor_size(){
    QListWidgetItem* current_theme_item = nullptr;
    for (int i = 0; i < cursor_theme_list->count(); ++i) {
        QListWidgetItem* item = cursor_theme_list->item(i);
        if (item->checkState() == Qt::Checked) {
            current_theme_item = item;
            break;
        }
    }
    if(current_theme_item)
        set_cursor_theme(current_theme_item);
}

void CursorThemeSettings::set_cursor_theme(QListWidgetItem *item){
    int size = size_input->value();
    QString theme = item->text();

    QString dirPath = QDir::home().path() + QStringLiteral("/.icons/default");
    QDir().mkpath(dirPath); // ensure the existence of the ~/.icons/default dir
    QFile indexTheme(dirPath + QStringLiteral("/index.theme"));
    if(indexTheme.open(QIODevice::WriteOnly|QIODevice::Truncate)){
        QTextStream(&indexTheme) <<
        "[Icon Theme]\n" <<
        "Name=Default\n" <<
        "Comment=Default cursor theme\n" <<
        "Inherits=" << theme << "\n" <<
        "Size=" << size << "\n";
        indexTheme.close();
    }

    // The compositor watches index.theme; GTK on Wayland reads GSettings instead.
    QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.interface", "cursor-theme", theme});
    QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.interface", "cursor-size", QString::number(size)});
}
