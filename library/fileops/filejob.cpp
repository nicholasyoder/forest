// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filejob.h"
#include "fileops.h"

#include <QAtomicInt>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSemaphore>
#include <QThread>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <optional>
#include <sys/stat.h>
#include <unistd.h>

namespace fileops {
namespace {

constexpr qint64 ChunkSize = 1 << 20;
constexpr QDir::Filters AllEntries = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

bool isRealDir(const QFileInfo &fi) { return fi.isDir() && !fi.isSymLink(); }
bool entryExists(const QFileInfo &fi) { return fi.exists() || fi.isSymLink(); }

// Resolves symlinks in the parent only, so a symlink entry stays itself.
QString canonicalEntry(const QString &path)
{
    const QFileInfo fi(path);
    const QString parent = QFileInfo(fi.absolutePath()).canonicalFilePath();
    return parent.isEmpty() ? QDir::cleanPath(fi.absoluteFilePath()) : QDir(parent).filePath(fi.fileName());
}

// True if path is root itself or anything beneath it.
bool isInside(const QString &path, const QString &root)
{
    const QString p = canonicalEntry(path), r = canonicalEntry(root);
    return p == r || p.startsWith(r + QLatin1Char('/'));
}

// Same directory by inode, so symlinked and bind-mounted paths match too.
bool sameDir(const QString &a, const QString &b)
{
    struct stat sa, sb;
    return ::stat(QFile::encodeName(a).constData(), &sa) == 0
        && ::stat(QFile::encodeName(b).constData(), &sb) == 0
        && sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

// Same directory entry (not merely a hardlink to the same file).
bool sameEntry(const QFileInfo &a, const QFileInfo &b)
{
    return a.fileName() == b.fileName() && sameDir(a.absolutePath(), b.absolutePath());
}

class QtFileJob : public FileJob
{
public:
    QtFileJob(Op op, const QStringList &sources, const QString &destDir)
        : FileJob(op), m_sources(sources), m_destDir(QDir::cleanPath(destDir)) {}

    ~QtFileJob() override
    {
        if (m_thread && m_thread->isRunning()) {
            cancel();
            m_thread->wait();
        }
    }

    void start() override
    {
        m_thread = QThread::create([this]{ run(); });
        m_thread->setParent(this);
        // Emitted from the GUI thread once the worker is done with all members.
        connect(m_thread, &QThread::finished, this, [this]{
            emit finished(m_failed, m_errors, m_cancel.loadRelaxed());
        });
        m_thread->start();
    }

    // Always releases: a spare count is harmless since ask() returns Cancel from now on.
    void cancel() override
    {
        m_cancel.storeRelaxed(1);
        m_reply.release();
    }

    void resolveConflict(Resolution resolution, bool applyToAll) override
    {
        m_answer = resolution;
        m_applyToAll = applyToAll;
        m_reply.release();
    }

private:
    enum Result { Ok, Partial, Failed }; // ordered by severity; Partial = skipped or cancelled
    enum class Plan { Write, Replace, Merge, Skip, Fail, Done }; // Replace: dst is a non-folder to swap atomically

    void run()
    {
        scan();
        emit progress(m_done, m_total, QString());
        for (const QString &src : m_sources) {
            if (cancelled())
                break;
            switch (op()) {
            case Op::Copy: copyTop(src); break;
            case Op::Move: moveTop(src); break;
            case Op::Delete: if (!removeEntry(src, true) && !cancelled()) m_failed << src; break;
            case Op::Trash: trashOne(src); break;
            }
        }
    }

    bool cancelled() const { return m_cancel.loadRelaxed(); }

    Result fail(const QString &path, const QString &why)
    {
        m_errors << QStringLiteral("%1: %2").arg(path, why);
        return Failed;
    }

    void report(const QString &current)
    {
        if (m_tick.isValid() && m_tick.elapsed() < 100)
            return;
        m_tick.start();
        emit progress(m_done, m_total, current);
    }

    void scan()
    {
        for (const QString &src : m_sources) {
            if (cancelled())
                return;
            switch (op()) {
            case Op::Copy: m_total += treeSize(src, false); break;
            case Op::Move: if (!sameDevice(src, m_destDir)) m_total += treeSize(src, false); break;
            case Op::Delete: m_total += treeSize(src, true); break;
            case Op::Trash: m_total += 1; break;
            }
        }
    }

    // Bytes of regular files, or entry count, without following symlinks.
    qint64 treeSize(const QString &path, bool countItems) const
    {
        const QFileInfo fi(path);
        if (!isRealDir(fi))
            return countItems ? 1 : (fi.isFile() && !fi.isSymLink() ? fi.size() : 0);
        qint64 n = countItems ? 1 : 0;
        QDirIterator it(path, AllEntries, QDirIterator::Subdirectories);
        while (it.hasNext() && !cancelled()) {
            const QFileInfo c = it.nextFileInfo();
            n += countItems ? 1 : (c.isFile() && !c.isSymLink() ? c.size() : 0);
        }
        return n;
    }

    // noOverwrite: a remembered Overwrite doesn't apply, so the user is asked again.
    Resolution ask(const QString &src, const QString &dst, bool noOverwrite)
    {
        if (m_remembered && !(noOverwrite && *m_remembered == Resolution::Overwrite))
            return *m_remembered;
        if (cancelled())
            return Resolution::Cancel;
        emit conflict(src, dst);
        m_reply.acquire();
        if (cancelled())
            return Resolution::Cancel;
        if (m_applyToAll)
            m_remembered = m_answer;
        return m_answer;
    }

    // Settles an existing dst: may pick a new name (Rename) or ask to replace it (Overwrite).
    Plan prepareDest(const QString &src, QString &dst, bool srcIsDir)
    {
        const QFileInfo d(dst);
        if (!entryExists(d))
            return Plan::Write;
        // dst is src under another path: Overwrite would delete the source.
        if (sameEntry(QFileInfo(src), d)) {
            if (op() == Op::Move)
                return Plan::Done;
            dst = uniquePath(d.absolutePath(), d.fileName(), QStringLiteral("copy"));
            return Plan::Write;
        }
        if (isInside(src, dst)) {
            fail(src, QStringLiteral("would replace a folder containing it"));
            return Plan::Fail;
        }
        // Never replace a folder with a non-folder: that would delete its whole tree.
        const bool dstIsFolder = isRealDir(d);
        switch (ask(src, dst, dstIsFolder && !srcIsDir)) {
        case Resolution::Skip:
            return Plan::Skip;
        case Resolution::Cancel:
            m_cancel.storeRelaxed(1);
            return Plan::Skip;
        case Resolution::Rename:
            dst = uniquePath(d.absolutePath(), d.fileName(), QString());
            return Plan::Write;
        case Resolution::Overwrite:
            if (dstIsFolder) {
                if (srcIsDir)
                    return Plan::Merge;
                fail(src, QStringLiteral("a folder with the same name already exists"));
                return Plan::Fail;
            }
            // A folder can't be renamed over a file, so that case still removes first.
            if (srcIsDir)
                return removeEntry(dst, false) ? Plan::Write : Plan::Fail;
            return Plan::Replace;
        }
        return Plan::Fail;
    }

    void copyTop(const QString &src)
    {
        const QFileInfo fi(src);
        QString dst = m_destDir + QLatin1Char('/') + fi.fileName();
        Result r;
        if (!entryExists(fi))
            r = fail(src, QStringLiteral("no longer exists"));
        else if (sameDir(fi.absolutePath(), m_destDir))
            r = copyEntry(src, uniquePath(m_destDir, fi.fileName(), QStringLiteral("copy")));
        else if (isRealDir(fi) && isInside(m_destDir, src))
            r = fail(src, QStringLiteral("can't copy a folder into itself"));
        else
            r = copyEntry(src, dst);
        if (r == Failed)
            m_failed << src;
    }

    Result copyEntry(const QString &src, QString dst)
    {
        if (cancelled())
            return Partial;
        const QFileInfo fi(src);
        switch (prepareDest(src, dst, isRealDir(fi))) {
        case Plan::Skip: return Partial;
        case Plan::Fail: return Failed;
        case Plan::Done: return Ok;
        case Plan::Write: case Plan::Merge: return copyPrepared(src, dst, false);
        case Plan::Replace: return copyPrepared(src, dst, true);
        }
        return Failed;
    }

    // dst is settled; replace: it exists as a non-folder and is swapped in only on success.
    Result copyPrepared(const QString &src, const QString &dst, bool replace)
    {
        const QFileInfo fi(src);
        if (fi.isSymLink()) {
            const QString tmp = replace ? tempSibling(dst) : dst;
            if (!QFile::link(fi.readSymLink(), tmp))
                return fail(dst, QStringLiteral("couldn't create link"));
            return replace ? commitReplace(tmp, dst) : Ok;
        }
        if (fi.isDir())
            return copyDir(src, dst, fi);
        if (!fi.isFile())
            return fail(src, QStringLiteral("special files can't be copied"));
        return copyFile(src, dst, fi, replace);
    }

    // Hidden temp name next to dst, so the final rename stays on one filesystem.
    static QString tempSibling(const QString &dst)
    {
        const QFileInfo d(dst);
        QString p;
        for (int i = 0; entryExists(QFileInfo(p = d.absolutePath() + QStringLiteral("/.%1.%2.part").arg(d.fileName()).arg(i))); ++i) {}
        return p;
    }

    Result commitReplace(const QString &tmp, const QString &dst)
    {
        const QByteArray t = QFile::encodeName(tmp), d = QFile::encodeName(dst);
        if (::rename(t.constData(), d.constData()) == 0)
            return Ok;
        const QString why = QString::fromLocal8Bit(std::strerror(errno));
        ::unlink(t.constData());
        return fail(dst, why);
    }

    Result copyDir(const QString &src, const QString &dst, const QFileInfo &fi)
    {
        // Merging into an existing folder keeps its mode; a new one starts private.
        const bool created = !QFileInfo(dst).isDir();
        if (created && ::mkdir(QFile::encodeName(dst).constData(), 0700) != 0)
            return fail(dst, QStringLiteral("couldn't create folder"));
        Result r = Ok;
        for (const QString &name : QDir(src).entryList(AllEntries)) {
            if (cancelled())
                return Partial;
            r = std::max(r, copyEntry(src + QLatin1Char('/') + name, dst + QLatin1Char('/') + name));
        }
        // After the children, so a read-only source folder doesn't block them.
        if (created)
            QFile::setPermissions(dst, fi.permissions());
        return r;
    }

    Result copyFile(const QString &src, const QString &finalDst, const QFileInfo &fi, bool replace)
    {
        const QString dst = replace ? tempSibling(finalDst) : finalDst;
        QFile in(src), out;
        if (!in.open(QIODevice::ReadOnly))
            return fail(src, in.errorString());
        // Owner-only until the data is in; final mode is set at the end.
        const int fd = ::open(QFile::encodeName(dst).constData(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
        if (fd < 0)
            return fail(dst, QString::fromLocal8Bit(std::strerror(errno)));
        out.setFileName(dst);
        if (!out.open(fd, QIODevice::WriteOnly | QIODevice::Unbuffered, QFileDevice::AutoCloseHandle))
            return fail(dst, out.errorString());

        QByteArray buf(ChunkSize, Qt::Uninitialized);
        for (;;) {
            if (cancelled()) {
                out.close();
                ::unlink(QFile::encodeName(dst).constData());
                return Partial;
            }
            const qint64 n = in.read(buf.data(), buf.size());
            if (n == 0)
                break;
            if (n < 0) {
                out.close();
                ::unlink(QFile::encodeName(dst).constData());
                return fail(src, in.errorString());
            }
            if (out.write(buf.constData(), n) != n) {
                const QString why = out.errorString();
                out.close();
                ::unlink(QFile::encodeName(dst).constData());
                return fail(dst, why);
            }
            m_done += n;
            report(fi.fileName());
        }
        out.setPermissions(fi.permissions());
        out.setFileTime(fi.lastModified(), QFileDevice::FileModificationTime);
        out.close();
        return replace ? commitReplace(dst, finalDst) : Ok;
    }

    void moveTop(const QString &src)
    {
        const QFileInfo fi(src);
        Result r;
        if (!entryExists(fi))
            r = fail(src, QStringLiteral("no longer exists"));
        else if (sameDir(fi.absolutePath(), m_destDir))
            return; // already there
        else if (isRealDir(fi) && isInside(m_destDir, src))
            r = fail(src, QStringLiteral("can't move a folder into itself"));
        else
            r = moveEntry(src, m_destDir + QLatin1Char('/') + fi.fileName());
        if (r == Failed)
            m_failed << src;
    }

    Result moveEntry(const QString &src, QString dst)
    {
        if (cancelled())
            return Partial;
        const QFileInfo fi(src);
        const Plan plan = prepareDest(src, dst, isRealDir(fi));
        switch (plan) {
        case Plan::Skip: return Partial;
        case Plan::Fail: return Failed;
        case Plan::Done: return Ok;
        case Plan::Write: case Plan::Replace: break;
        case Plan::Merge: {
            Result r = Ok;
            for (const QString &name : QDir(src).entryList(AllEntries)) {
                if (cancelled())
                    return Partial;
                r = std::max(r, moveEntry(src + QLatin1Char('/') + name, dst + QLatin1Char('/') + name));
            }
            if (r == Ok && !QDir().rmdir(src))
                return fail(src, QStringLiteral("couldn't remove folder"));
            return r;
        }
        }

        // QDir::rename hides errno, and QFile::rename silently falls back to an uncancellable copy.
        const QByteArray s = QFile::encodeName(src), d = QFile::encodeName(dst);
        int rc = ::renameat2(AT_FDCWD, s.constData(), AT_FDCWD, d.constData(), RENAME_NOREPLACE);
        if (plan == Plan::Replace)
            rc = ::rename(s.constData(), d.constData()); // atomic over the old file
        else if (rc != 0 && errno == EINVAL) // filesystem without RENAME_NOREPLACE; dst was checked above
            rc = ::rename(s.constData(), d.constData());
        if (rc == 0) {
            report(fi.fileName());
            return Ok;
        }
        if (errno != EXDEV)
            return fail(src, QString::fromLocal8Bit(std::strerror(errno)));

        const Result r = copyPrepared(src, dst, plan == Plan::Replace);
        if (r != Ok)
            return r;
        if (removeEntry(src, false))
            return Ok;
        return cancelled() ? Partial : Failed;
    }

    // Recursive delete without following symlinks; reports its own errors.
    bool removeEntry(const QString &path, bool countProgress)
    {
        if (cancelled())
            return false;
        const QFileInfo fi(path);
        if (isRealDir(fi)) {
            bool ok = true;
            for (const QString &name : QDir(path).entryList(AllEntries))
                ok = removeEntry(path + QLatin1Char('/') + name, countProgress) && ok;
            if (!ok)
                return false;
            if (!QDir().rmdir(path)) {
                fail(path, QStringLiteral("couldn't remove folder"));
                return false;
            }
        } else {
            QFile f(path);
            if (!f.remove()) {
                fail(path, f.errorString());
                return false;
            }
        }
        if (countProgress) {
            ++m_done;
            report(fi.fileName());
        }
        return true;
    }

    void trashOne(const QString &path)
    {
        // No error message: the UI offers a permanent delete for m_failed instead.
        if (!QFile::moveToTrash(path))
            m_failed << path;
        ++m_done;
        report(QFileInfo(path).fileName());
    }

    const QStringList m_sources;
    const QString m_destDir;
    QThread *m_thread = nullptr;

    // Worker-thread state; read by the GUI thread only after QThread::finished.
    QStringList m_failed, m_errors;
    qint64 m_done = 0, m_total = 0;
    QElapsedTimer m_tick;
    std::optional<Resolution> m_remembered;

    QAtomicInt m_cancel;
    QSemaphore m_reply;
    Resolution m_answer = Resolution::Cancel; // handed over via m_reply
    bool m_applyToAll = false;
};

}

FileJob *FileJob::create(Op op, const QStringList &sources, const QString &destDir)
{
    return new QtFileJob(op, sources, destDir);
}

}
