// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopview.h"
#include "fileops.h"
#include "menuanchor.h"

#include <QApplication>
#include <QAbstractFileIconProvider>
#include <QCollator>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDrag>
#include <QFileSystemModel>
#include <QHoverEvent>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QMenu>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPainter>
#include <QPaintDeviceWindow>
#include <QTextLayout>
#include <QTimer>
#include <QtMath>
#include <QWindow>
#include <LayerShellQt/Window>
#include <qt6xdg/XdgDesktopFile>
#include <qt6xdg/XdgMimeType>
#include <algorithm>

namespace {

constexpr char positionsgroup[] = "desktopicons";

// desktopmodel supplies icons on the UI thread; skip the gatherer thread's lookups.
class noiconprovider : public QAbstractFileIconProvider
{
public:
    QIcon icon(IconType) const override { return QIcon(); }
    QIcon icon(const QFileInfo &) const override { return QIcon(); }
};

// Qt paints the drag icon synchronously when showing it, before start_drag and the
// hotspot offset, so it flashes with its corner at the cursor. Repaint it once both are sent.
class dragiconfix : public QObject
{
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::Paint || !watched->inherits("QShapedPixmapWindow"))
            return false;
        auto *window = static_cast<QPaintDeviceWindow*>(watched);
        QTimer::singleShot(0, window, [window](){ window->update(); });
        qApp->removeEventFilter(this);
        return true;
    }
};

// Black copy of `src` with its alpha box-blurred (radius r), averaging in-bounds pixels only.
QImage shadowof(const QImage &src, int r)
{
    const int w = src.width(), h = src.height();
    QList<int> a(w*h), t(w*h);
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        for (int x = 0; x < w; ++x)
            a[y*w + x] = qAlpha(line[x]);
    }
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int sum = 0, n = 0;
            for (int i = qMax(0, x - r); i <= qMin(w - 1, x + r); ++i, ++n)
                sum += a[y*w + i];
            t[y*w + x] = sum / n;
        }
    QImage out(w, h, QImage::Format_ARGB32_Premultiplied);
    out.setDevicePixelRatio(src.devicePixelRatio());
    for (int y = 0; y < h; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < w; ++x) {
            int sum = 0, n = 0;
            for (int j = qMax(0, y - r); j <= qMin(h - 1, y + r); ++j, ++n)
                sum += t[j*w + x];
            line[x] = qRgba(0, 0, 0, sum / n);
        }
    }
    return out;
}

}

// desktopmodel ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

desktopmodel::desktopmodel(QFileSystemModel *source, QObject *parent) : QIdentityProxyModel(parent)
{
    source->setIconProvider(new noiconprovider);
    setSourceModel(source);
    connect(source, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &tl, const QModelIndex &br){
        for (int row = tl.row(); row <= br.row(); ++row)
            cache.remove(tl.siblingAtRow(row).data(QFileSystemModel::FilePathRole).toString());
    });
    connect(source, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this, source](const QModelIndex &parent, int first, int last){
        for (int row = first; row <= last; ++row)
            cache.remove(source->filePath(source->index(row, 0, parent)));
    });
}

const desktopmodel::entry &desktopmodel::lookup(const QModelIndex &index) const
{
    const QString path = index.data(QFileSystemModel::FilePathRole).toString();
    auto it = cache.find(path);
    if (it != cache.end())
        return *it;

    entry e;
    const QFileInfo info(path); // fresh: the model's copy can lag a chmod
    const XdgMimeType mime(QMimeDatabase().mimeTypeForFile(info));
    XdgDesktopFile dfile;
    // Untrusted launchers show their real filename, not a self-chosen Name/Icon.
    if (mime.inherits("application/x-desktop") && info.isExecutable() && dfile.load(path)) {
        e.name = dfile.name();
        e.icon = dfile.icon();
    }
    if (e.name.isEmpty())
        e.name = info.fileName();
    if (e.icon.isNull())
        e.icon = mime.icon();
    return *cache.insert(path, e);
}

QVariant desktopmodel::data(const QModelIndex &index, int role) const
{
    if (index.column() == 0) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: return lookup(index).name;
        case Qt::DecorationRole: return lookup(index).icon;
        }
    }
    return QIdentityProxyModel::data(index, role);
}

Qt::ItemFlags desktopmodel::flags(const QModelIndex &index) const
{
    // Editable for the rename editor only; the source stays read-only so it never renames or accepts drops itself.
    return QIdentityProxyModel::flags(index) | Qt::ItemIsEditable;
}

void desktopmodel::refresh(const QString &path)
{
    if (path.isEmpty())
        cache.clear();
    else
        cache.remove(path);
    auto *fs = static_cast<QFileSystemModel*>(sourceModel());
    const QModelIndex root = mapFromSource(fs->index(fs->rootPath()));
    if (rowCount(root) > 0)
        emit dataChanged(index(0, 0, root), index(rowCount(root) - 1, 0, root));
}

// renameedit ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

renameedit::renameedit(QWidget *parent) : QTextEdit(parent)
{
    setObjectName("desktopIconRename");
    setAcceptRichText(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QTextOption option(Qt::AlignHCenter);
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    document()->setDefaultTextOption(option);
    document()->setDocumentMargin(2);
    connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &renameedit::fit);
}

void renameedit::fit()
{
    const int h = qCeil(document()->size().height()) + 2*frameWidth();
    if (h != height())
        resize(width(), h);
}

void renameedit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        emit done();
        return;
    }
    QTextEdit::keyPressEvent(event);
}

void renameedit::insertFromMimeData(const QMimeData *source)
{
    insertPlainText(source->text().simplified());
}

// desktopdelegate ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void desktopdelegate::seticonsize(int size)
{
    if (m_iconsize == size)
        return;
    m_iconsize = size;
    labels.clear();
}

void desktopdelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget *w = opt.widget;
    QStyle *style = w ? w->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w); // QSS ::item

    const QRect r = opt.rect;
    opt.icon.paint(painter, QRect(r.x() + (r.width() - m_iconsize)/2, r.y() + 2, m_iconsize, m_iconsize));
    if (index == editing)
        return;
    const int top = labeltop();
    painter->drawPixmap(r.x(), r.y() + top,
                        label(opt.text, QSize(r.width(), r.height() - top), opt.font, painter->device()->devicePixelRatioF()));
}

// White text, word-wrapped and elided to fit, over a blurred shadow; cached since the blur is costly.
QPixmap desktopdelegate::label(const QString &text, QSize size, const QFont &font, qreal dpr) const
{
    const QString key = text + '\n' + QString::number(size.width()) + 'x' + QString::number(size.height())
                        + '@' + QString::number(dpr) + font.key();
    if (QPixmap *cached = labels.object(key))
        return *cached;

    const QFontMetrics fm(font);
    const int spacing = fm.lineSpacing();
    QTextOption textoption(Qt::AlignCenter);
    textoption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    QTextLayout layout(text, font);
    layout.setTextOption(textoption);

    QList<QTextLine> lines;
    QString elided;
    layout.beginLayout();
    forever {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(size.width());
        if (size.height() >= (lines.size() + 2) * spacing) {
            lines.append(line);
        } else {
            elided = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());
            break;
        }
    }
    layout.endLayout();

    const int count = lines.size() + (elided.isEmpty() ? 0 : 1);
    const int top = (size.height() - count*spacing) / 2;

    QImage textimg(size * dpr, QImage::Format_ARGB32_Premultiplied);
    textimg.setDevicePixelRatio(dpr);
    textimg.fill(Qt::transparent);
    {
        QPainter p(&textimg);
        p.setFont(font);
        p.setPen(Qt::white);
        for (int i = 0; i < lines.size(); ++i)
            lines[i].draw(&p, QPointF(0, top + i*spacing));
        if (!elided.isEmpty())
            p.drawText(QRectF(0, top + lines.size()*spacing, size.width(), spacing), elided, QTextOption(Qt::AlignCenter));
    }

    QPixmap result = QPixmap::fromImage(shadowof(textimg, qRound(2*dpr)));
    {
        QPainter p(&result);
        p.drawImage(0, 0, textimg);
    }
    labels.insert(key, new QPixmap(result));
    return result;
}

QWidget *desktopdelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto *edit = new renameedit(parent);
    connect(edit, &renameedit::done, this, [this, edit](){
        emit const_cast<desktopdelegate*>(this)->commitData(edit);
        emit const_cast<desktopdelegate*>(this)->closeEditor(edit);
    });
    editing = index;
    return edit;
}

void desktopdelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    editing = QPersistentModelIndex();
    QStyledItemDelegate::destroyEditor(editor, index);
}

void desktopdelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *edit = static_cast<renameedit*>(editor);
    edit->setPlainText(index.data(QFileSystemModel::FileNameRole).toString());
    edit->selectAll();
}

void desktopdelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
    if (onrename)
        onrename(index.data(QFileSystemModel::FilePathRole).toString(), static_cast<renameedit*>(editor)->toPlainText().simplified());
}

void desktopdelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    auto *edit = static_cast<renameedit*>(editor);
    edit->document()->setTextWidth(option.rect.width() - 2*edit->frameWidth());
    edit->setGeometry(option.rect.x(), option.rect.y() + labeltop(), option.rect.width(),
                      qCeil(edit->document()->size().height()) + 2*edit->frameWidth());
}

// desktopview ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

desktopview::desktopview(desktopmodel *model, const QString &dir, QScreen *screen) : dir(dir)
{
    setObjectName("desktopIcons");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    viewport()->setAttribute(Qt::WA_Hover);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setViewMode(QListView::IconMode);
    setMovement(QListView::Free);
    setUniformItemSizes(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionRectVisible(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers); // rename via the F2 action
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setAutoScroll(false);
    setItemDelegate(new desktopdelegate(this));

    setModel(model);
    auto *fs = static_cast<QFileSystemModel*>(model->sourceModel());
    setRootIndex(model->mapFromSource(fs->index(dir)));
    loadpositions();

    winId(); // force native window creation so windowHandle() is valid
    windowHandle()->setScreen(screen);
    resize(screen->size()); // a 0x0 top-level never maps; the compositor sends the real size
    LayerShellQt::Window *layer = LayerShellQt::Window::get(windowHandle());
    layer->setLayer(LayerShellQt::Window::LayerBottom);
    layer->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                      | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
    layer->setExclusiveZone(0); // sized to the usable area, clear of panels
    layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
    layer->setScope("forest-desktop-icons");
}

void desktopview::applysettings(const iconsettings &cfg)
{
    config = cfg;
    static_cast<desktopdelegate*>(itemDelegate())->seticonsize(config.iconsize);
    setIconSize(QSize(config.iconsize, config.iconsize));
    if (!config.singleclick)
        viewport()->unsetCursor();
    doItemsLayout();
    viewport()->update();
}

int desktopview::columns() const
{
    return qMax(1, (viewport()->width() - 2*margin + config.gridspacing) / cellx());
}

int desktopview::rows() const
{
    return qMax(1, (viewport()->height() - 2*margin + config.gridspacing) / celly());
}

QPoint desktopview::cellat(QPoint contentpos) const
{
    auto floordiv = [](int a, int b){ return a >= 0 ? a / b : -((-a + b - 1) / b); };
    return QPoint(floordiv(contentpos.x() - margin, cellx()), floordiv(contentpos.y() - margin, celly()));
}

QList<QModelIndex> desktopview::sortedindexes(const QList<QModelIndex> &indexes) const
{
    struct sortitem {
        QModelIndex index;
        QString name;
        bool isdir = false;
        QString type;
        qint64 size = 0;
        qint64 modified = 0;
    };

    const bool bytype = config.sortby == QLatin1String("type");
    const bool bysize = config.sortby == QLatin1String("size");
    const bool bymod = config.sortby == QLatin1String("modified");
    const bool desc = config.sortorder == QLatin1String("descending");
    QMimeDatabase db;

    QList<sortitem> items;
    items.reserve(indexes.size());
    for (const QModelIndex &index : indexes) {
        const QFileInfo info(index.data(QFileSystemModel::FilePathRole).toString());
        sortitem s;
        s.index = index;
        s.name = index.data(Qt::DisplayRole).toString();
        s.isdir = info.isDir();
        if (bytype)
            s.type = s.isdir ? QString() : db.mimeTypeForFile(info).comment();
        else if (bysize)
            s.size = s.isdir ? 0 : info.size();
        else if (bymod)
            s.modified = info.lastModified().toMSecsSinceEpoch();
        items.append(std::move(s));
    }

    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);

    std::stable_sort(items.begin(), items.end(), [&](const sortitem &a, const sortitem &b) {
        if (config.foldersfirst && a.isdir != b.isdir)
            return a.isdir;
        int cmp = 0;
        if (bytype)
            cmp = collator.compare(a.type, b.type);
        else if (bysize)
            cmp = (a.size < b.size) ? -1 : (a.size > b.size ? 1 : 0);
        else if (bymod)
            cmp = (a.modified < b.modified) ? -1 : (a.modified > b.modified ? 1 : 0);
        else
            cmp = collator.compare(a.name, b.name);

        if (cmp != 0)
            return desc ? (cmp > 0) : (cmp < 0);
        return collator.compare(a.name, b.name) < 0;
    });

    QList<QModelIndex> out;
    out.reserve(items.size());
    for (const sortitem &s : std::as_const(items))
        out.append(s.index);
    return out;
}

QString desktopview::nameof(const QModelIndex &index) const
{
    return index.data(QFileSystemModel::FileNameRole).toString();
}

QStringList desktopview::selectedpaths() const
{
    QStringList paths;
    for (const QModelIndex &index : selectedIndexes())
        paths.append(index.data(QFileSystemModel::FilePathRole).toString());
    return paths;
}

void desktopview::renameselected()
{
    const QModelIndexList selected = selectedIndexes();
    if (selected.size() == 1)
        edit(selected.first());
}

// Files appear asynchronously; edit once the model reports it.
void desktopview::editwhenadded(const QString &path)
{
    pendingedit = path;
}

void desktopview::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    if (pendingedit.isEmpty() || parent != rootIndex())
        return;
    for (int row = start; row <= end; ++row) {
        const QModelIndex index = model()->index(row, 0, parent);
        if (index.data(QFileSystemModel::FilePathRole).toString() == pendingedit) {
            pendingedit.clear();
            QTimer::singleShot(0, this, [this, p = QPersistentModelIndex(index)](){
                if (p.isValid()) {
                    setCurrentIndex(p);
                    edit(p);
                }
            });
            return;
        }
    }
}

void desktopview::renamed(const QString &oldname, const QString &newname)
{
    if (!stored.contains(oldname))
        return;
    const QPoint c = stored.value(oldname);
    unstore(oldname);
    store(newname, c);
}

// Positions --------------------------------------------------------------------

void desktopview::loadpositions()
{
    settings.beginGroup(positionsgroup);
    for (const QString &name : settings.childKeys()) {
        const QStringList parts = settings.value(name).toString().split(',');
        if (parts.size() == 2)
            stored.insert(name, QPoint(parts[0].toInt(), parts[1].toInt()));
    }
    settings.endGroup();
}

void desktopview::store(const QString &name, QPoint c)
{
    stored.insert(name, c);
    settings.setValue(QString(positionsgroup) + "/" + name, QString::number(c.x()) + "," + QString::number(c.y()));
}

void desktopview::unstore(const QString &name)
{
    stored.remove(name);
    settings.remove(QString(positionsgroup) + "/" + name);
}

// QListView re-lays out every item on any row change; put them back on their cells.
void desktopview::doItemsLayout()
{
    QListView::doItemsLayout();
    placeitems();
}

void desktopview::arrangeicons()
{
    if (!config.autoarrange) {
        stored.clear();
        settings.beginGroup(positionsgroup);
        settings.remove("");
        settings.endGroup();
    }
    placeitems();
}

void desktopview::placeitems()
{
    QAbstractItemModel *m = model();
    if (!m)
        return;
    const QModelIndex root = rootIndex();

    placed.clear();
    occupant.clear();
    auto put = [this](const QModelIndex &index, const QString &name, QPoint c){
        placed.insert(name, c);
        occupant.insert(c, name);
        const QPoint pos = cellpos(c);
        if (rectForIndex(index).topLeft() != pos)
            setPositionForIndex(pos, index);
    };

    const int rowcount = rows();
    if (config.autoarrange) {
        QList<QModelIndex> all;
        all.reserve(m->rowCount(root));
        for (int row = 0; row < m->rowCount(root); ++row)
            all.append(m->index(row, 0, root));
        QPoint next(0, 0);
        for (const QModelIndex &index : sortedindexes(all)) {
            put(index, nameof(index), next);
            next.ry()++;
            if (next.y() >= rowcount)
                next = QPoint(next.x() + 1, 0);
        }
        return;
    }

    // Stored cells stay reserved, so new icons never take a displaced (or not yet loaded) icon's spot.
    QSet<QPoint> reserved;
    for (auto it = stored.cbegin(); it != stored.cend(); ++it)
        if (inside(it.value()))
            reserved.insert(it.value());

    QList<QModelIndex> rest;
    for (int row = 0; row < m->rowCount(root); ++row) {
        const QModelIndex index = m->index(row, 0, root);
        const QString name = nameof(index);
        auto it = stored.constFind(name);
        if (it != stored.cend() && inside(*it) && !occupant.contains(*it))
            put(index, name, *it);
        else
            rest.append(index);
    }

    // Column-major from the top left; past the last column when the grid is full.
    QPoint next(0, 0);
    for (const QModelIndex &index : sortedindexes(rest)) {
        while (occupant.contains(next) || reserved.contains(next)) {
            next.ry()++;
            if (next.y() >= rowcount)
                next = QPoint(next.x() + 1, 0);
        }
        const QString name = nameof(index);
        put(index, name, next);
        // Out-of-grid or colliding stored cells are kept for when they fit again.
        if (canstore && !stored.contains(name) && inside(next))
            store(name, next);
    }

    if (canstore) {
        const QStringList names = stored.keys();
        for (const QString &name : names)
            if (!placed.contains(name) && !QFileInfo::exists(dir + "/" + name))
                unstore(name);
    }
}

void desktopview::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);
    scheduleDelayedItemsLayout();
}

void desktopview::paintEvent(QPaintEvent *event)
{
    if (!canstore) {
        canstore = true; // painting means the compositor's configure has been applied
        scheduleDelayedItemsLayout();
    }
    QListView::paintEvent(event);
}

// QWidget::event() update()s the whole viewport on hover enter/leave; the base class already
// repaints just the hovered item, so skip that full-surface repaint.
bool desktopview::viewportEvent(QEvent *event)
{
    const bool handled = QListView::viewportEvent(event);
    if (config.singleclick && event->type() == QEvent::HoverMove) {
        const QPoint pos = static_cast<QHoverEvent*>(event)->position().toPoint();
        if (indexAt(pos).isValid())
            viewport()->setCursor(Qt::PointingHandCursor);
        else
            viewport()->unsetCursor();
    } else if (config.singleclick && event->type() == QEvent::HoverLeave) {
        viewport()->unsetCursor();
    }
    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave)
        return true;
    return handled;
}

// Input ------------------------------------------------------------------------

void desktopview::mousePressEvent(QMouseEvent *event)
{
    presspos = event->position().toPoint();
    pressedindex = (event->button() == Qt::LeftButton) ? indexAt(presspos) : QModelIndex();
    draggedpress = false;
    QListView::mousePressEvent(event);
}

// Starts the drag right away instead of after the platform's drag distance.
void desktopview::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex pressed = indexAt(presspos);
    if ((event->buttons() & Qt::LeftButton) && state() == NoState && pressed.isValid()
        && selectionModel()->isSelected(pressed)
        && (event->position().toPoint() - presspos).manhattanLength() >= 3) {
        draggedpress = true;
        setState(DraggingState);
        startDrag(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
        setState(NoState);
        return;
    }
    QListView::mouseMoveEvent(event);
}

void desktopview::mouseReleaseEvent(QMouseEvent *event)
{
    QListView::mouseReleaseEvent(event);
    if (config.singleclick && !draggedpress && event->button() == Qt::LeftButton
        && event->modifiers() == Qt::NoModifier && state() != EditingState) {
        const QModelIndex idx = indexAt(event->position().toPoint());
        if (idx.isValid() && idx == pressedindex)
            emit itemactivated(idx);
    }
    pressedindex = QPersistentModelIndex();
    draggedpress = false;
}

void desktopview::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!config.singleclick && event->button() == Qt::LeftButton) {
        const QModelIndex idx = indexAt(event->position().toPoint());
        if (idx.isValid()) {
            emit itemactivated(idx);
            return;
        }
    }
    QListView::mouseDoubleClickEvent(event);
}

void desktopview::keyPressEvent(QKeyEvent *event)
{
    const bool enter = event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter;
    if (enter && state() != EditingState && !selectedIndexes().isEmpty()) {
        emit openrequested();
        return;
    }
    QListView::keyPressEvent(event);
}

void desktopview::contextMenuEvent(QContextMenuEvent *event)
{
    // The press already selected the icon under the cursor (or cleared the selection).
    QMenu *menu = indexAt(event->pos()).isValid() ? iconmenu : deskmenu;
    if (!menu)
        return;
    menuanchor::anchorMenuAtPoint(menu, viewport(), event->pos());
    menu->exec(event->globalPos());
}

// Drag and drop ----------------------------------------------------------------

void desktopview::startDrag(Qt::DropActions)
{
    const QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty())
        return;

    draganchor = indexAt(presspos);
    if (!draganchor.isValid())
        draganchor = indexes.first();
    dragoffset = presspos - visualRect(draganchor).topLeft();
    dragged.clear();
    QRect bounds;
    for (const QModelIndex &index : indexes) {
        dragged.append(index);
        bounds |= visualRect(index);
    }

    const qreal dpr = devicePixelRatioF();
    QPixmap pixmap(bounds.size() * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);
    {
        QPainter p(&pixmap);
        for (const QModelIndex &index : indexes) {
            QStyleOptionViewItem opt;
            initViewItemOption(&opt);
            opt.rect = visualRect(index).translated(-bounds.topLeft());
            opt.state |= QStyle::State_Selected;
            itemDelegate()->paint(&p, opt, index);
        }
    }

    auto *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(indexes));
    drag->setPixmap(pixmap);
    drag->setHotSpot(presspos - bounds.topLeft());
    // An external Move (e.g. to a file manager) moves the files; the model drops them.
    dragiconfix fix;
    qApp->installEventFilter(&fix);
    drag->exec(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction, Qt::MoveAction);
    qApp->removeEventFilter(&fix);
    dragged.clear();
}

// Re-evaluated on every move: the drag source sees the action and modifiers can change mid-drag.
bool desktopview::acceptdrop(QDropEvent *event)
{
    if (event->source() == this) {
        if (config.autoarrange) {
            event->ignore();
            return false;
        }
        event->setDropAction(Qt::MoveAction);
        event->accept();
        return true;
    }
    const QStringList paths = fileops::localPaths(event->mimeData()->urls());
    const Qt::DropAction action = paths.isEmpty() ? Qt::IgnoreAction
        : fileops::dropAction(paths, dir, event->modifiers(), event->possibleActions());
    if (action == Qt::IgnoreAction) {
        event->ignore();
        return false;
    }
    event->setDropAction(action);
    event->accept();
    return true;
}

void desktopview::dragEnterEvent(QDragEnterEvent *event)
{
    acceptdrop(event);
}

void desktopview::dragMoveEvent(QDragMoveEvent *event)
{
    acceptdrop(event);
}

void desktopview::dropEvent(QDropEvent *event)
{
    if (!acceptdrop(event))
        return;
    if (event->source() == this)
        movedragged(event->position().toPoint());
    else
        emit filesdropped(fileops::localPaths(event->mimeData()->urls()), event->dropAction(),
                          event->mimeData()->hasFormat(fileops::trustedLauncherMime));
}

// Shifts the dragged icons by the anchor's cell offset, if every target cell is in the grid and free.
void desktopview::movedragged(QPoint droppos)
{
    if (!draganchor.isValid() || config.autoarrange)
        return;
    const QPoint offset(horizontalOffset(), verticalOffset());
    const QSize sz = itemsize();
    const QPoint center(sz.width()/2, sz.height()/2);
    const QPoint delta = cellat(droppos + offset - dragoffset + center) - placed.value(nameof(draganchor));
    if (delta.isNull())
        return;

    QSet<QString> moving;
    for (const QPersistentModelIndex &index : std::as_const(dragged))
        if (index.isValid())
            moving.insert(nameof(index));
    for (const QString &name : std::as_const(moving)) {
        const QPoint target = placed.value(name) + delta;
        const QString other = occupant.value(target);
        if (!inside(target) || (!other.isEmpty() && !moving.contains(other)))
            return;
    }
    for (const QString &name : std::as_const(moving))
        store(name, placed.value(name) + delta);
    placeitems();
}
