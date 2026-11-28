// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <QListView>
#include <QIdentityProxyModel>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QCache>
#include <QSettings>
#include <QPersistentModelIndex>
#include <functional>

class QFileSystemModel;
class QMenu;

// Executable launchers show their Name/Icon; everything else its filename and mime icon.
class desktopmodel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit desktopmodel(QFileSystemModel *source, QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    // Re-reads launchers (all when path is empty), e.g. after marking one executable.
    void refresh(const QString &path = QString());

private:
    struct entry { QString name; QIcon icon; };
    const entry &lookup(const QModelIndex &index) const;
    mutable QHash<QString, entry> cache;
};

// Wrapping rename box that grows downwards; Enter commits, newlines never get in.
class renameedit : public QTextEdit
{
    Q_OBJECT
public:
    explicit renameedit(QWidget *parent);
signals:
    void done();
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;
private:
    void fit();
};

class desktopdelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    static constexpr int itemsize = 96;
    static constexpr int iconsize = 48;
    static constexpr int labeltop = 46;

    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override { return {itemsize, itemsize}; }
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    // Renames through `onrename` instead of the model (Forest's own error dialogs).
    void setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;

    std::function<void(const QString &path, const QString &newname)> onrename;

private:
    QPixmap label(const QString &text, QSize size, const QFont &font, qreal dpr) const;
    mutable QCache<QString, QPixmap> labels{512};
    mutable QPersistentModelIndex editing; // its label is hidden under the editor
};

// Icon grid on its own Bottom-layer surface (exclusive zone 0), so its size is the
// output's usable area. Positions are grid cells stored by filename.
class desktopview : public QListView
{
    Q_OBJECT
public:
    desktopview(desktopmodel *model, const QString &dir, QScreen *screen);

    void setmenus(QMenu *icons, QMenu *desktop) { iconmenu = icons; deskmenu = desktop; }
    QStringList selectedpaths() const;
    void renameselected();
    void editwhenadded(const QString &path);
    void renamed(const QString &oldname, const QString &newname);

    void doItemsLayout() override;

signals:
    void filesdropped(QStringList paths, Qt::DropAction action, bool trusted);
    void openrequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void startDrag(Qt::DropActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void rowsInserted(const QModelIndex &parent, int start, int end) override;

private:
    static constexpr int margin = 10;
    static constexpr int cell = 100;

    int columns() const;
    int rows() const;
    bool inside(QPoint c) const { return c.x() >= 0 && c.y() >= 0 && c.x() < columns() && c.y() < rows(); }
    QPoint cellpos(QPoint c) const { return QPoint(margin + c.x()*cell, margin + c.y()*cell); }
    QPoint cellat(QPoint contentpos) const;
    QString nameof(const QModelIndex &index) const;

    void placeitems();
    void store(const QString &name, QPoint c);
    void unstore(const QString &name);
    void loadpositions();
    bool acceptdrop(QDropEvent *event);
    void movedragged(QPoint droppos);

    QString dir;
    QSettings settings{"Forest", "Forest"};
    QHash<QString, QPoint> stored;   // name -> cell, from settings
    QHash<QString, QPoint> placed;   // name -> cell in the current layout
    QHash<QPoint, QString> occupant; // cell -> name in the current layout
    bool canstore = false;           // the compositor's size is known (first paint)

    QPoint presspos;
    QList<QPersistentModelIndex> dragged;
    QPersistentModelIndex draganchor;
    QPoint dragoffset;
    QString pendingedit;

    QMenu *iconmenu = nullptr;
    QMenu *deskmenu = nullptr;
};

#endif // DESKTOPVIEW_H
