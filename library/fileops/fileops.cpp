// SPDX-License-Identifier: LGPL-3.0-or-later

#include "fileops.h"
#include "filejob.h"

#include <QCheckBox>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <sys/stat.h>

namespace fileops {
namespace {

const QString GnomeFormat = QStringLiteral("x-special/gnome-copied-files");
const QString KdeCutFormat = QStringLiteral("application/x-kde-cutselection");

QString itemsText(const QStringList &paths)
{
    return paths.size() == 1 ? QStringLiteral("“%1”").arg(QFileInfo(paths.first()).fileName())
                             : QStringLiteral("%1 items").arg(paths.size());
}

void askConflict(FileJob *job, const QString &src, const QString &dst)
{
    const QFileInfo s(src), d(dst);
    const bool merge = s.isDir() && !s.isSymLink() && d.isDir() && !d.isSymLink();

    auto *box = new QMessageBox(QMessageBox::Question,
                                merge ? QStringLiteral("Folder already exists") : QStringLiteral("File already exists"),
                                QStringLiteral("“%1” already exists in “%2”.").arg(d.fileName(), d.absolutePath()));
    box->setInformativeText(merge ? QStringLiteral("Merge the folders, replacing files with the same name?")
                                  : QStringLiteral("Replace it?"));
    QAbstractButton *overwrite = box->addButton(merge ? QStringLiteral("Merge") : QStringLiteral("Replace"),
                                                QMessageBox::DestructiveRole);
    QAbstractButton *rename = box->addButton(QStringLiteral("Keep both"), QMessageBox::AcceptRole);
    QAbstractButton *skip = box->addButton(QStringLiteral("Skip"), QMessageBox::RejectRole);
    box->addButton(QMessageBox::Cancel);
    box->setDefaultButton(qobject_cast<QPushButton *>(rename));
    box->setCheckBox(new QCheckBox(QStringLiteral("Apply to all")));
    box->setAttribute(Qt::WA_DeleteOnClose);

    QPointer<FileJob> guard(job);
    QObject::connect(job, &QObject::destroyed, box, &QObject::deleteLater);
    // finished, not buttonClicked: also fires for Esc / window close (clickedButton() is then Cancel).
    QObject::connect(box, &QDialog::finished, box, [=]{
        if (!guard)
            return;
        QAbstractButton *b = box->clickedButton();
        const Resolution r = b == overwrite ? Resolution::Overwrite
                           : b == rename ? Resolution::Rename
                           : b == skip ? Resolution::Skip
                           : Resolution::Cancel;
        guard->resolveConflict(r, box->checkBox()->isChecked());
    });
    box->open();
}

// Progress window appears only for jobs still running after 500 ms.
void run(FileJob *job, const QString &title, const QString &errorText,
         std::function<void(const QStringList &)> onFailed = {})
{
    auto *dlg = new QDialog;
    job->setParent(dlg);
    dlg->setWindowTitle(title);
    dlg->setMinimumWidth(400);

    auto *label = new QLabel;
    label->setTextFormat(Qt::PlainText);
    auto *bar = new QProgressBar;
    bar->setRange(0, 0);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
    auto *layout = new QVBoxLayout(dlg);
    layout->addWidget(label);
    layout->addWidget(bar);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
    QObject::connect(dlg, &QDialog::rejected, job, &FileJob::cancel);
    QObject::connect(job, &FileJob::progress, dlg, [label, bar](qint64 done, qint64 total, const QString &current){
        label->setText(current);
        if (total > 0) {
            bar->setRange(0, 1000);
            bar->setValue(int(std::min(done, total) * 1000 / total));
        }
    });
    QObject::connect(job, &FileJob::conflict, dlg, [job](const QString &src, const QString &dst){
        askConflict(job, src, dst);
    });
    QObject::connect(job, &FileJob::finished, dlg,
                     [dlg, errorText, onFailed](const QStringList &failed, const QStringList &errors, bool cancelled){
        dlg->hide();
        dlg->deleteLater();
        if (!errors.isEmpty())
            showErrors(errorText, errors);
        if (onFailed && !failed.isEmpty() && !cancelled)
            onFailed(failed);
    });
    QTimer::singleShot(500, dlg, &QWidget::show);
    job->start();
}

void startDelete(const QStringList &paths)
{
    run(FileJob::create(Op::Delete, paths), QStringLiteral("Deleting files"),
        QStringLiteral("Some items couldn't be deleted."));
}

void confirmDelete(const QStringList &paths, const QString &text)
{
    auto *box = new QMessageBox(QMessageBox::Warning, QStringLiteral("Delete permanently"), text);
    box->setInformativeText(QStringLiteral("This can't be undone."));
    QAbstractButton *del = box->addButton(QStringLiteral("Delete"), QMessageBox::DestructiveRole);
    box->addButton(QMessageBox::Cancel);
    box->setDefaultButton(QMessageBox::Cancel);
    box->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(box, &QDialog::finished, box, [box, del, paths]{
        if (box->clickedButton() == del)
            startDelete(paths);
    });
    box->open();
}

}

void copy(const QStringList &sources, const QString &destDir)
{
    run(FileJob::create(Op::Copy, sources, destDir), QStringLiteral("Copying files"),
        QStringLiteral("Some items couldn't be copied."));
}

void move(const QStringList &sources, const QString &destDir)
{
    run(FileJob::create(Op::Move, sources, destDir), QStringLiteral("Moving files"),
        QStringLiteral("Some items couldn't be moved."));
}

void trash(const QStringList &paths)
{
    run(FileJob::create(Op::Trash, paths), QStringLiteral("Moving to trash"),
        QStringLiteral("Some items couldn't be moved to the trash."), [](const QStringList &failed){
        confirmDelete(failed, QStringLiteral("%1 can't be moved to the trash. Delete permanently?").arg(itemsText(failed)));
    });
}

void remove(const QStringList &paths)
{
    if (!paths.isEmpty())
        confirmDelete(paths, QStringLiteral("Permanently delete %1?").arg(itemsText(paths)));
}

void setClipboard(const QStringList &paths, bool cut)
{
    QList<QUrl> urls;
    QByteArray gnome = cut ? "cut" : "copy";
    for (const QString &path : paths) {
        urls << QUrl::fromLocalFile(path);
        gnome += '\n' + urls.last().toEncoded();
    }
    auto *data = new QMimeData;
    data->setUrls(urls);
    data->setData(GnomeFormat, gnome);
    if (cut)
        data->setData(KdeCutFormat, "1");
    QGuiApplication::clipboard()->setMimeData(data);
}

void paste(const QString &destDir)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *data = clipboard->mimeData();
    if (!data)
        return;

    QStringList paths;
    bool cut = false;
    if (data->hasFormat(GnomeFormat)) {
        QList<QByteArray> lines = data->data(GnomeFormat).split('\n');
        cut = lines.takeFirst().trimmed() == "cut";
        for (const QByteArray &line : lines) {
            const QUrl url = QUrl::fromEncoded(line.trimmed());
            if (url.isLocalFile())
                paths << url.toLocalFile();
        }
    } else if (data->hasUrls()) {
        paths = localPaths(data->urls());
        cut = data->data(KdeCutFormat).startsWith('1');
    }
    if (paths.isEmpty())
        return;

    if (cut) {
        move(paths, destDir);
        clipboard->clear();
    } else {
        copy(paths, destDir);
    }
}

Qt::DropAction dropAction(const QStringList &sources, const QString &destDir,
                          Qt::KeyboardModifiers modifiers, Qt::DropActions allowed)
{
    Qt::DropAction action;
    if (modifiers & Qt::ControlModifier)
        action = Qt::CopyAction;
    else if (modifiers & Qt::ShiftModifier)
        action = Qt::MoveAction;
    else
        action = std::all_of(sources.begin(), sources.end(), [&](const QString &s){ return sameDevice(s, destDir); })
                 ? Qt::MoveAction : Qt::CopyAction;

    if (allowed & action)
        return action;
    const Qt::DropAction other = action == Qt::CopyAction ? Qt::MoveAction : Qt::CopyAction;
    return allowed & other ? other : Qt::IgnoreAction;
}

QStringList localPaths(const QList<QUrl> &urls)
{
    QStringList paths;
    for (const QUrl &url : urls) {
        if (url.isLocalFile())
            paths << url.toLocalFile();
    }
    return paths;
}

bool sameDevice(const QString &path, const QString &dir)
{
    struct stat a, b;
    return ::lstat(QFile::encodeName(path).constData(), &a) == 0
        && ::stat(QFile::encodeName(dir).constData(), &b) == 0
        && a.st_dev == b.st_dev;
}

QString uniquePath(const QString &dir, const QString &name, const QString &tag)
{
    QString base = name, ext;
    const QFileInfo fi(dir + QLatin1Char('/') + name);
    if (!fi.isDir() || fi.isSymLink()) {
        static const QRegularExpression suffix(QStringLiteral("(\\.tar)?\\.[^.]+$"));
        const QRegularExpressionMatch m = suffix.match(name);
        if (m.hasMatch() && m.capturedStart() > 0) {
            base = name.left(m.capturedStart());
            ext = m.captured();
        }
    }
    for (int n = 1;; ++n) {
        const QString label = tag.isEmpty() ? QString::number(n + 1)
                            : n == 1 ? tag : QStringLiteral("%1 %2").arg(tag).arg(n);
        const QString candidate = QStringLiteral("%1/%2 (%3)%4").arg(dir, base, label, ext);
        const QFileInfo c(candidate);
        if (!c.exists() && !c.isSymLink())
            return candidate;
    }
}

void showErrors(const QString &text, const QStringList &errors)
{
    constexpr int Shown = 5;
    auto *box = new QMessageBox(QMessageBox::Warning, QStringLiteral("File operation failed"), text);
    QString info = errors.mid(0, Shown).join(QLatin1Char('\n'));
    if (errors.size() > Shown) {
        info += QStringLiteral("\n…and %1 more").arg(errors.size() - Shown);
        box->setDetailedText(errors.join(QLatin1Char('\n')));
    }
    box->setInformativeText(info);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->open();
}

}
