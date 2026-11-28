// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QColor>
#include <QFileSystemWatcher>
#include <QImage>
#include <QObject>
#include <QPointer>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>
#include <memory>

#include "wallpaperconfig.h"

namespace wallpaper {

class FadeOverlay;

// Image of exact targetSize filled with background and source painted according to mode.
QImage render(const QImage &source, Mode mode, QSize targetSize, const QColor &background = Qt::black);

// Decodes the active wallpaper once and drives slideshow cycling so all screens stay in sync.
class WallpaperSource : public QObject {
    Q_OBJECT
public:
    explicit WallpaperSource(QObject *parent = nullptr);

    void setConfig(const Config &config, const QString &fallbackImage = {});
    const Config &config() const { return m_config; }

    bool isSolidColor() const { return m_config.type == Type::SolidColor; }
    QColor backgroundColor() const { return m_config.color; }
    Mode mode() const { return m_config.mode; }
    const QImage &currentImage() const { return m_image; }
    QString currentImagePath() const { return m_currentPath; }
    int slideshowCount() const { return m_files.size(); }

public slots:
    void next();

signals:
    void changed(bool animate = false);
    void fadeUpdated(qreal opacity);
    void fadeFinished();

private:
    void loadImageFile(const QString &path);
    void loadFallback();
    void watchDirectory(const QString &dirPath);
    void rescanDirectory(bool keepCurrent, bool emitIfChanged);
    void buildOrder();
    bool advance(bool emitSignal);
    void advanceAsync();
    void updateTimer();
    void emitChanged(bool animate);
    void startFadeNow();
    void registerFadeWarmup();
    void fadeWarmupDone();

    Config m_config;
    QString m_fallbackImage;
    QImage m_image;
    QString m_currentPath;
    QStringList m_files;
    QVector<int> m_order;
    int m_index = -1;
    quint64 m_loadGen = 0;
    QTimer m_timer;
    QTimer m_rescanTimer;
    QTimer m_warmupFallback;
    QFileSystemWatcher m_watcher;
    QVariantAnimation m_fade;
    int m_warmupPending = 0;

    friend class WallpaperView;
};

// Paints a WallpaperSource, caching one scaled buffer per widget size.
class WallpaperView : public QWidget {
    Q_OBJECT
public:
    explicit WallpaperView(WallpaperSource *source = nullptr, QWidget *parent = nullptr);
    ~WallpaperView() override;

    void setSource(WallpaperSource *source);
    WallpaperSource *source() const { return m_source; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QSize physicalSize() const;
    void renderScaled(QSize physSize);
    void clearFade();
    void onSourceChanged(bool animate);
    void onFadeUpdated(qreal opacity);
    void onFadeFinished();

    QPointer<WallpaperSource> m_source;
    QImage m_scaled;
    QImage m_previous;
    std::unique_ptr<FadeOverlay> m_fadeOverlay;
    qreal m_fadeOpacity = 1.0;
};

} // namespace wallpaper

#endif // WALLPAPER_H
