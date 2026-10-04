// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>
#include <QStringList>

namespace fileops {

enum class Op { Copy, Move, Delete, Trash };
enum class Resolution { Overwrite, Skip, Rename, Cancel };

// Backend-neutral job API; create() picks the backend (pure Qt today).
class FileJob : public QObject
{
    Q_OBJECT

public:
    static FileJob *create(Op op, const QStringList &sources, const QString &destDir = QString());

    Op op() const { return m_op; }
    virtual void start() = 0;
    virtual void cancel() = 0;
    // Answers conflict(); the job is blocked until this is called.
    virtual void resolveConflict(Resolution resolution, bool applyToAll) = 0;

signals:
    void progress(qint64 done, qint64 total, const QString &current);
    void conflict(const QString &source, const QString &dest);
    // failed: top-level sources not processed (for Trash: couldn't be trashed).
    void finished(const QStringList &failed, const QStringList &errors, bool cancelled);

protected:
    explicit FileJob(Op op) : m_op(op) {}

private:
    Op m_op;
};

}
#endif // FILEJOB_H
