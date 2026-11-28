// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WALLPAPEREDITOR_H
#define WALLPAPEREDITOR_H

#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>

#include "settings_plugin_interface.h"
#include "wallpaperconfig.h"

class ColorButton;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QWidget;

// Shared wallpaper settings UI (preview + background type, image/folder, mode, color, slideshow).
class WallpaperEditor : public QObject {
    Q_OBJECT
public:
    struct Options {
        bool allowSlideshow = true;
        QString backgroundDescription;
        // Optional label formatter for the chosen image path; empty falls back to filename.
        std::function<QString(const QString &)> formatImageName;
        // Optional start dir override for the photo browser when current path is internal.
        std::function<QString(const QString &)> photoBrowseDir;
    };

    WallpaperEditor(settings_page *page, const Options &options, QObject *parent = nullptr);

    void setConfig(const wallpaper::Config &config);
    wallpaper::Config config() const { return m_config; }

signals:
    // Emitted on user edits; `control` identifies the settings row, `immediate` is false while typing.
    void changed(QWidget *control, bool immediate);

private:
    enum IntervalUnit { Seconds = 1, Minutes = 60, Hours = 3600 };

    void choosePhoto();
    void chooseFolder();
    void updateIntervalRange();
    void applyIntervalFromUi(bool immediate);
    void setIntervalUi(int seconds);
    void rescanFolder();
    void syncUi();
    void updatePreview();
    void emitChanged(QWidget *control, bool immediate);

    Options m_options;
    wallpaper::Config m_config;
    bool m_loading = false;

    QLabel *m_preview = nullptr;
    QComboBox *m_typeBox = nullptr;
    QWidget *m_imageControl = nullptr;
    QLabel *m_imageLabel = nullptr;
    QPushButton *m_browsePhoto = nullptr;
    QWidget *m_folderControl = nullptr;
    QLabel *m_folderLabel = nullptr;
    QPushButton *m_browseFolder = nullptr;
    QComboBox *m_modeBox = nullptr;
    ColorButton *m_colorButton = nullptr;
    QWidget *m_intervalControl = nullptr;
    QSpinBox *m_intervalSpin = nullptr;
    QComboBox *m_intervalUnit = nullptr;
    QCheckBox *m_shuffleCheck = nullptr;

    QString m_previewPath;
    QImage m_previewSource;
    QStringList m_folderImages;
};

#endif // WALLPAPEREDITOR_H
