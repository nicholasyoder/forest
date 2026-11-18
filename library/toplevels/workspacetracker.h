// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WORKSPACETRACKER_H
#define WORKSPACETRACKER_H

#include <QObject>
#include <QString>

class ExtWorkspaceManager;
class BiomeWorkspaces;

// ext-workspace (list, switching) plus org.biome.Workspaces (which window is
// on which workspace). Workspaces are addressed by list index; org.biome
// numbers are the ext-workspace id parsed as an integer, or the arrival index
// when a compositor sends no numeric id.
class WorkspaceTracker : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceTracker(QObject *parent = nullptr);
    ~WorkspaceTracker();

    int count() const;
    QString name(int index) const;
    // -1 until the compositor has sent the list.
    int activeIndex() const;
    void activate(int index);

    // The window's workspace index; -1 if unknown (not classified yet, or
    // not running on Biome).
    int indexOfWindow(const QString &identifier) const;
    // Windows per workspace index.
    QList<int> windowCounts() const;
    // Moving windows needs org.biome.Workspaces.
    bool canMoveWindows() const;
    void moveWindow(const QString &identifier, int index);

signals:
    // The workspace list, active workspace or window map changed.
    void changed();

private:
    int numberOf(int index) const;
    int indexOfNumber(int number) const;

    ExtWorkspaceManager *m_manager = nullptr;
    BiomeWorkspaces *m_biome = nullptr;
};

#endif // WORKSPACETRACKER_H
