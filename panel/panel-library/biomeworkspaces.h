// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BIOMEWORKSPACES_H
#define BIOMEWORKSPACES_H

#include <QObject>
#include <QVariantMap>

// Async client for org.biome.Workspaces: the window -> workspace mapping
// that ext-workspace-v1 lacks. Keyed by ext-foreign-toplevel-list identifier.
class BiomeWorkspaces : public QObject {
    Q_OBJECT

public:
    explicit BiomeWorkspaces(QObject *parent = nullptr);

    // Checked once at construction: Biome owns org.biome before it starts
    // the session, so false means another compositor.
    bool isAvailable() const { return m_available; }
    const QVariantMap &windowWorkspaces() const { return m_windowWorkspaces; }

    void moveToplevel(const QString &identifier, int workspace);

signals:
    void windowWorkspacesChanged();

private slots:
    void onWindowWorkspacesChanged(const QVariantMap &windowWorkspaces);

private:
    bool m_available = false;
    QVariantMap m_windowWorkspaces;
};

#endif // BIOMEWORKSPACES_H
