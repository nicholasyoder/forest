#include "cursorthemesettings.h"

#include <QDir>
#include <QSettings>
#include <QProcess>

#include <QtX11Extras/QX11Info>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

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
    settings_item = new settings_category("Mouse Cursor", "", "preferences-desktop-mouse");
    connect(settings_item, &settings_category::opened, this, &CursorThemeSettings::load_cursor_themes);

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

    Display* dpy = QX11Info::display();
    QString current_theme = QString::fromUtf8(XcursorGetTheme(dpy));
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
    size_input->setValue(XcursorGetDefaultSize(dpy));
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

    Display* dpy = QX11Info::display();
    XcursorSetTheme(dpy, qPrintable(theme));
    XcursorSetDefaultSize(dpy, size);

    // Reload the standard cursors
    QStringList names;
    names << "left_ptr"            << "up_arrow"             << "cross"           << "wait"
          << "left_ptr_watch"      << "ibeam"                << "size_ver"        << "size_hor"
          << "size_bdiag"          << "size_fdiag"           << "size_all"        << "split_v"
          << "split_h"             << "pointing_hand"        << "openhand"        << "text"
          << "closedhand"          << "forbidden"            << "whats_this"      << "pirate"
          << "X_cursor"            << "right_ptr"            << "hand1"           << "question_arrow"
          << "hand2"               << "watch"                << "xterm"           << "left_side"
          << "crosshair"           << "left_ptr_watch"       << "center_ptr"      << "right_side"
          << "sb_h_double_arrow"   << "sb_v_double_arrow"    << "fleur"           << "top_side"
          << "top_left_corner"     << "top_right_corner"     << "bottom_side"
          << "bottom_left_corner"  << "bottom_right_corner";
    foreach(QString name, names){
        QByteArray cursorName = QFile::encodeName(name);
        QByteArray themeName  = QFile::encodeName(theme);
        XcursorImages *images = XcursorLibraryLoadImages(cursorName.constData(), themeName.constData(), size);
        if (images){
            unsigned long cursor_handle = 0;
            cursor_handle = (unsigned long)XcursorImagesLoadCursor(dpy, images);
            XcursorImagesDestroy(images);
            XFixesChangeCursorByName(dpy, cursor_handle, QFile::encodeName(name).constData());
        }
    }

    set_x_cursor_in_file(QDir::home().path() + QStringLiteral("/.Xresources"), theme, size);
    set_x_cursor_in_file(QDir::home().path() + QStringLiteral("/.Xdefaults"), theme, size);

    QProcess xrdb;
    xrdb.start(QStringLiteral("xrdb"), QStringList() << QStringLiteral("-merge") << QDir::home().path() + QStringLiteral("/.Xresources"));
    xrdb.waitForFinished();

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
}

// The contents of this function are copied from LxQt
void CursorThemeSettings::set_x_cursor_in_file(QString file, QString theme, int size){
    QStringList lst;
    {
        QFile fl(file);
        if (fl.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&fl);
            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.startsWith(QLatin1String("Xcursor.theme:"))
                    && !line.startsWith(QLatin1String("Xcursor.size:")))
                {
                    lst << line;
                }
            }
            fl.close();
        }
    }
    while (lst.size() > 0)
    {
        QString s(lst[lst.size()-1]);
        if (!s.trimmed().isEmpty()) break;
        lst.removeAt(lst.size()-1);
    }
    {
        QFile fl(file);
        if (fl.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&fl);
            for (const QString &s : std::as_const(lst))
            {
                stream << s << "\n";
            }
            stream << "\nXcursor.theme: " << theme << "\n";
            stream << "Xcursor.size: " << size << "\n";
            fl.close();
        }
    }
}
