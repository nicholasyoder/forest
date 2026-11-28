// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaper.h"
#include "fadeoverlay.h"

#include <QCoreApplication>
#include <QDir>
#include <QEasingCurve>
#include <QMetaObject>
#include <QPainter>
#include <QRandomGenerator>
#include <QThreadPool>
#include <QtMath>
#include <algorithm>
#include <numeric>

namespace wallpaper {

namespace {

constexpr int RescanDebounceMs = 500;
constexpr int FadeDurationMs = 500;
constexpr int WarmupFallbackMs = 120;

QColor opaque(const QColor &color)
{
    return color.isValid() ? QColor(color.red(), color.green(), color.blue()) : QColor(Qt::black);
}

} // namespace

QImage render(const QImage &source, Mode mode, QSize targetSize, const QColor &background)
{
    if (targetSize.isEmpty())
        return {};

    QImage out(targetSize, QImage::Format_RGB32);
    out.fill(opaque(background));
    if (source.isNull())
        return out;

    QImage image;
    QRectF target, src;
    const int tw = targetSize.width();
    const int th = targetSize.height();

    switch (mode) {
    case Mode::Fill: {
        image = source.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int x = (image.width() - tw) / 2;
        const int y = (image.height() - th) / 2;
        target = QRectF(0, 0, tw, th);
        src = QRectF(x, y, tw, th);
        break;
    }
    case Mode::Fit: {
        image = source.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        target = QRectF((tw - image.width()) / 2, (th - image.height()) / 2, image.width(), image.height());
        src = QRectF(image.rect());
        break;
    }
    case Mode::Stretch: {
        image = source.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        target = QRectF(0, 0, tw, th);
        src = QRectF(image.rect());
        break;
    }
    case Mode::Center: {
        image = source;
        target = QRectF((tw - image.width()) / 2, (th - image.height()) / 2, image.width(), image.height());
        src = QRectF(image.rect());
        break;
    }
    }

    QPainter painter(&out);
    painter.drawImage(target, image, src);
    return out;
}

WallpaperSource::WallpaperSource(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &WallpaperSource::advanceAsync);

    m_rescanTimer.setSingleShot(true);
    m_rescanTimer.setInterval(RescanDebounceMs);
    connect(&m_rescanTimer, &QTimer::timeout, this, [this] { rescanDirectory(true, true); });

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        m_rescanTimer.start();
    });

    m_fade.setStartValue(0.0);
    m_fade.setEndValue(1.0);
    m_fade.setDuration(FadeDurationMs);
    m_fade.setEasingCurve(QEasingCurve::InOutQuad);
    connect(&m_fade, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        emit fadeUpdated(value.toReal());
        FadeOverlay::flushDisplay();
    });
    connect(&m_fade, &QVariantAnimation::finished, this, &WallpaperSource::fadeFinished);

    m_warmupFallback.setSingleShot(true);
    m_warmupFallback.setInterval(WarmupFallbackMs);
    connect(&m_warmupFallback, &QTimer::timeout, this, &WallpaperSource::startFadeNow);
}

void WallpaperSource::emitChanged(bool animate)
{
    m_fade.stop();
    m_warmupFallback.stop();
    m_warmupPending = 0;
    // Views pre-scale (and set up GPU fade overlays) synchronously during changed().
    emit changed(animate);
    if (!animate)
        return;
    if (m_warmupPending > 0)
        m_warmupFallback.start();
    else
        m_fade.start();
}

void WallpaperSource::registerFadeWarmup()
{
    ++m_warmupPending;
}

void WallpaperSource::fadeWarmupDone()
{
    if (m_warmupPending <= 0)
        return;
    if (--m_warmupPending == 0)
        startFadeNow();
}

void WallpaperSource::startFadeNow()
{
    m_warmupFallback.stop();
    m_warmupPending = 0;
    if (m_fade.state() != QAbstractAnimation::Running)
        m_fade.start();
}

void WallpaperSource::setConfig(const Config &config, const QString &fallbackImage)
{
    ++m_loadGen;
    const Type oldType = m_config.type;
    const QString oldDir = m_config.directory;
    const bool oldShuffle = m_config.shuffle;

    m_config = config;
    m_config.color = opaque(m_config.color);
    m_config.intervalSeconds = clampInterval(m_config.intervalSeconds);
    m_fallbackImage = fallbackImage;

    if (m_config.type == Type::SolidColor) {
        m_timer.stop();
        m_rescanTimer.stop();
        watchDirectory({});
        m_files.clear();
        m_order.clear();
        m_index = -1;
        m_image = {};
        m_currentPath.clear();
        emitChanged(false);
        return;
    }

    if (m_config.type == Type::Image) {
        m_timer.stop();
        m_rescanTimer.stop();
        watchDirectory({});
        m_files.clear();
        m_order.clear();
        m_index = -1;
        loadImageFile(m_config.imagePath);
        emitChanged(false);
        return;
    }

    // Type::Slideshow
    watchDirectory(m_config.directory);
    const bool sameDir = (oldType == Type::Slideshow && oldDir == m_config.directory);
    if (sameDir && oldShuffle == m_config.shuffle && !m_currentPath.isEmpty() && !m_image.isNull()) {
        updateTimer();
        emitChanged(false);
        return;
    }
    rescanDirectory(sameDir, false);
    emitChanged(false);
}

void WallpaperSource::next()
{
    if (m_config.type != Type::Slideshow || m_files.size() <= 1)
        return;
    advanceAsync();
    if (m_files.size() > 1)
        m_timer.start(m_config.intervalSeconds * 1000);
}

void WallpaperSource::advanceAsync()
{
    if (m_files.isEmpty()) {
        loadFallback();
        m_timer.stop();
        emitChanged(false);
        return;
    }

    const int count = m_order.size();
    int startIndex = m_index + 1;
    if (startIndex >= count) {
        startIndex = 0;
        if (m_config.shuffle && count > 1) {
            buildOrder();
            // Put the current image last so corrupt entries at the front can't repeat it back-to-back.
            const int curFile = m_files.indexOf(m_currentPath);
            const int curPos = curFile >= 0 ? m_order.indexOf(curFile) : -1;
            if (curPos >= 0 && curPos != count - 1)
                std::swap(m_order[curPos], m_order[count - 1]);
        }
    }
    // Optimistically advance index so rapid next() clicks step forward instead of re-decoding the same file.
    m_index = startIndex;

    const quint64 gen = ++m_loadGen;
    const QStringList files = m_files;
    const QVector<int> order = m_order;
    QPointer<WallpaperSource> self(this);

    QThreadPool::globalInstance()->start([self, gen, files, order, startIndex] {
        const int n = order.size();
        int foundIndex = -1;
        QString foundPath;
        QImage foundImage;
        for (int attempt = 0; attempt < n; ++attempt) {
            const int idx = (startIndex + attempt) % n;
            const QString &candidate = files[order[idx]];
            QImage img(candidate);
            if (!img.isNull()) {
                foundIndex = idx;
                foundPath = candidate;
                foundImage = std::move(img);
                break;
            }
        }
        if (QCoreApplication *app = QCoreApplication::instance()) {
            QMetaObject::invokeMethod(app, [self, gen, foundIndex, foundPath, foundImage = std::move(foundImage)]() mutable {
                if (!self || self->m_loadGen != gen)
                    return;
                if (!foundImage.isNull()) {
                    const bool pathChanged = (foundPath != self->m_currentPath);
                    self->m_index = foundIndex;
                    self->m_currentPath = foundPath;
                    self->m_image = std::move(foundImage);
                    self->emitChanged(pathChanged);
                } else {
                    self->loadFallback();
                    self->m_timer.stop();
                    self->emitChanged(false);
                }
            }, Qt::QueuedConnection);
        }
    });
}

void WallpaperSource::loadImageFile(const QString &path)
{
    if (!path.isEmpty() && path == m_currentPath && !m_image.isNull())
        return;
    QImage img = path.isEmpty() ? QImage() : QImage(path);
    if (!img.isNull()) {
        m_currentPath = path;
        m_image = img;
        return;
    }
    loadFallback();
}

void WallpaperSource::loadFallback()
{
    if (m_fallbackImage.isEmpty()) {
        m_currentPath.clear();
        m_image = {};
        return;
    }
    if (m_currentPath == m_fallbackImage && !m_image.isNull())
        return;
    m_image = QImage(m_fallbackImage);
    m_currentPath = m_image.isNull() ? QString() : m_fallbackImage;
}

void WallpaperSource::watchDirectory(const QString &dirPath)
{
    const QStringList watched = m_watcher.directories();
    if (dirPath.isEmpty()) {
        if (!watched.isEmpty())
            m_watcher.removePaths(watched);
        return;
    }
    if (watched.size() == 1 && watched.first() == dirPath)
        return;
    if (!watched.isEmpty())
        m_watcher.removePaths(watched);
    if (QDir(dirPath).exists())
        m_watcher.addPath(dirPath);
}

void WallpaperSource::buildOrder()
{
    m_order.resize(m_files.size());
    std::iota(m_order.begin(), m_order.end(), 0);
    if (m_config.shuffle && m_order.size() > 1) {
        auto *rng = QRandomGenerator::global();
        for (int i = m_order.size() - 1; i > 0; --i)
            std::swap(m_order[i], m_order[rng->bounded(i + 1)]);
    }
}

void WallpaperSource::rescanDirectory(bool keepCurrent, bool emitIfChanged)
{
    ++m_loadGen;
    const QString prevPath = m_currentPath;
    m_files = listImagesInDirectory(m_config.directory);
    buildOrder();
    m_index = -1;

    if (m_files.isEmpty()) {
        loadFallback();
        updateTimer();
        if (emitIfChanged && m_currentPath != prevPath)
            emitChanged(true);
        return;
    }

    if (keepCurrent && !m_currentPath.isEmpty()) {
        const int fileIdx = m_files.indexOf(m_currentPath);
        if (fileIdx >= 0) {
            m_index = m_order.indexOf(fileIdx);
            if (m_image.isNull())
                m_image = QImage(m_currentPath);
            updateTimer();
            return;
        }
    }

    advance(false);
    updateTimer();
    if (emitIfChanged && m_currentPath != prevPath)
        emitChanged(true);
}

bool WallpaperSource::advance(bool emitSignal)
{
    if (m_files.isEmpty()) {
        loadFallback();
        m_timer.stop();
        if (emitSignal)
            emitChanged(false);
        return false;
    }

    const int count = m_order.size();
    for (int attempt = 0; attempt < count; ++attempt) {
        ++m_index;
        if (m_index >= count) {
            m_index = 0;
            if (m_config.shuffle && count > 1) {
                buildOrder();
                // Put the current image last so corrupt entries at the front can't repeat it back-to-back.
                const int curFile = m_files.indexOf(m_currentPath);
                const int curPos = curFile >= 0 ? m_order.indexOf(curFile) : -1;
                if (curPos >= 0 && curPos != count - 1)
                    std::swap(m_order[curPos], m_order[count - 1]);
            }
        }
        const QString &candidate = m_files[m_order[m_index]];
        QImage img(candidate);
        if (!img.isNull()) {
            const bool pathChanged = (candidate != m_currentPath);
            m_currentPath = candidate;
            m_image = img;
            if (emitSignal)
                emitChanged(pathChanged);
            return true;
        }
    }

    loadFallback();
    m_timer.stop();
    if (emitSignal)
        emitChanged(false);
    return false;
}

void WallpaperSource::updateTimer()
{
    if (m_config.type == Type::Slideshow && m_files.size() > 1) {
        const int ms = m_config.intervalSeconds * 1000;
        if (!m_timer.isActive() || m_timer.interval() != ms)
            m_timer.start(ms);
    } else {
        m_timer.stop();
    }
}

WallpaperView::WallpaperView(WallpaperSource *source, QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    FadeOverlay::initGlobals();
    setSource(source);
}

WallpaperView::~WallpaperView() = default;

void WallpaperView::setSource(WallpaperSource *source)
{
    if (m_source == source)
        return;
    if (m_source)
        disconnect(m_source, nullptr, this, nullptr);
    m_source = source;
    if (m_source) {
        connect(m_source, &WallpaperSource::changed, this, &WallpaperView::onSourceChanged);
        connect(m_source, &WallpaperSource::fadeUpdated, this, &WallpaperView::onFadeUpdated);
        connect(m_source, &WallpaperSource::fadeFinished, this, &WallpaperView::onFadeFinished);
        connect(m_source, &QObject::destroyed, this, [this] { onSourceChanged(false); });
    }
    onSourceChanged(false);
}

QSize WallpaperView::physicalSize() const
{
    const qreal dpr = devicePixelRatioF();
    return QSize(qCeil(width() * dpr), qCeil(height() * dpr));
}

void WallpaperView::renderScaled(QSize physSize)
{
    if (!m_source || m_source->isSolidColor() || m_source->currentImage().isNull() || physSize.isEmpty()) {
        m_scaled = {};
        return;
    }
    m_scaled = wallpaper::render(m_source->currentImage(), m_source->mode(), physSize, m_source->backgroundColor());
}

void WallpaperView::clearFade()
{
    m_fadeOverlay.reset();
    m_previous = {};
    m_fadeOpacity = 1.0;
}

void WallpaperView::onSourceChanged(bool animate)
{
    const QSize physSize = physicalSize();
    if (animate && isVisible() && !physSize.isEmpty() && !m_scaled.isNull() && m_scaled.size() == physSize) {
        if (!m_previous.isNull() && m_previous.size() == physSize) {
            // Preserve the current mid-fade blend if another slide starts before the last finishes.
            QPainter p(&m_previous);
            p.setOpacity(m_fadeOpacity);
            p.drawImage(QPoint(0, 0), m_scaled);
        } else {
            m_previous = m_scaled;
        }
        m_fadeOverlay.reset();
        // Pre-scale before the fade timer starts so scaling never eats fade frames.
        renderScaled(physSize);
        m_fadeOpacity = 0.0;

        // Prefer GPU alpha-modifier subsurface fade: zero per-frame SHM uploads.
        if (auto overlay = FadeOverlay::create(windowHandle(), m_previous, size())) {
            m_fadeOverlay = std::move(overlay);
            // Commit the new image on the main surface atomically underneath the opaque overlay.
            repaint();
            QPointer<WallpaperSource> src = m_source;
            if (src) {
                src->registerFadeWarmup();
                m_fadeOverlay->beginFade([src] {
                    if (src)
                        src->fadeWarmupDone();
                });
            } else {
                m_fadeOverlay->beginFade({});
            }
        }
        return;
    }

    clearFade();
    m_scaled = {};
    update();
}

void WallpaperView::onFadeUpdated(qreal opacity)
{
    if (m_fadeOverlay) {
        m_fadeOpacity = opacity;
        m_fadeOverlay->setOpacity(1.0 - opacity);
        return;
    }
    if (m_previous.isNull())
        return;
    m_fadeOpacity = opacity;
    update();
}

void WallpaperView::onFadeFinished()
{
    if (m_fadeOverlay) {
        clearFade();
        return;
    }
    if (m_previous.isNull())
        return;
    clearFade();
    update();
}

void WallpaperView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QSize physSize = physicalSize();
    const bool hasImage = m_source && !m_source->isSolidColor() && !m_source->currentImage().isNull();
    const QColor bg = m_source ? m_source->backgroundColor() : QColor(30, 30, 30);

    if (hasImage && m_scaled.size() != physSize)
        renderScaled(physSize);

    const bool cpuFading = !m_fadeOverlay && !m_previous.isNull() && m_previous.size() == physSize && m_fadeOpacity < 1.0;
    if (!cpuFading && (!hasImage || m_scaled.isNull())) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), bg);
        return;
    }

    // Undo Qt's fractional DPR transform so 1:1 physical buffers hit the SIMD blit/blend fast path.
    const qreal dpr = devicePixelRatioF();
    if (dpr != 1.0)
        painter.scale(1.0 / dpr, 1.0 / dpr);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    if (cpuFading) {
        painter.drawImage(QPoint(0, 0), m_previous);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(m_fadeOpacity);
    }

    if (!hasImage || m_scaled.isNull())
        painter.fillRect(QRect(QPoint(0, 0), physSize), bg);
    else
        painter.drawImage(QPoint(0, 0), m_scaled);
}

void WallpaperView::resizeEvent(QResizeEvent *)
{
    if (m_scaled.size() != physicalSize()) {
        clearFade();
        m_scaled = {};
    }
}

void WallpaperView::hideEvent(QHideEvent *event)
{
    clearFade();
    QWidget::hideEvent(event);
}

} // namespace wallpaper
