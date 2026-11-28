// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QColor>
#include <QFileSystemWatcher>
#include <QFont>
#include <QObject>
#include <QPalette>
#include <QString>
#include <QTimer>
#include <private/qgenericunixtheme_p.h>

class ForestPlatformTheme : public QGenericUnixTheme
{
public:
    ForestPlatformTheme();
    ~ForestPlatformTheme() override;

    const QPalette *palette(Palette type = SystemPalette) const override;
    const QFont *font(Font type = SystemFont) const override;
    QVariant themeHint(ThemeHint hint) const override;
    Qt::ColorScheme colorScheme() const override;
    QIcon fileIcon(const QFileInfo &fileInfo,
                   QPlatformTheme::IconOptions iconOptions = {}) const override;
    QIconEngine *createIconEngine(const QString &iconName) const override;

private:
    bool reloadConfig();
    void startWatcher();
    void watchConfigPaths();
    void onConfigChanged();

    static QString configDirPath();
    static QString configFilePath();

    QObject *m_helper = nullptr;
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_debounce = nullptr;

    QString m_themeName;
    QString m_iconTheme;
    Qt::ColorScheme m_colorScheme = Qt::ColorScheme::Dark;
    QPalette m_palette;
    bool m_hasPalette = false;
    QFont m_systemFont;
    QFont m_fixedFont;
};
