// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpapereditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardPaths>

#include "colorbutton.h"
#include "wallpaper.h"

namespace {

const QSize PreviewSize(400, 250);
constexpr int MaxPathLabelWidth = 260;

QString elided(QLabel *label, const QString &text)
{
    return label->fontMetrics().elidedText(text, Qt::ElideMiddle, MaxPathLabelWidth);
}

// Before SettingsManager reparents a control into its row, setVisible(true) would open it as a top-level window.
// Clearing ExplicitShowHide leaves it unshown until reparented, while RowSync treats it as visible.
void setRowVisible(QWidget *control, bool visible)
{
    if (control->parentWidget()) {
        control->setVisible(visible);
        return;
    }
    if (visible)
        control->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
    else
        control->hide();
}

} // namespace

WallpaperEditor::WallpaperEditor(settings_page *page, const Options &options, QObject *parent)
    : QObject(parent)
    , m_options(options)
{
    QFrame *previewBase = new QFrame;
    previewBase->setObjectName("WallpaperPreviewBase");
    QHBoxLayout *previewLayout = new QHBoxLayout(previewBase);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(0);
    previewLayout->addStretch(1);
    m_preview = new QLabel;
    m_preview->setObjectName("WallpaperPreviewImage");
    m_preview->setFixedSize(PreviewSize);
    previewLayout->addWidget(m_preview);
    previewLayout->addStretch(1);
    page->add_child(new settings_widget("", "", previewBase, true));

    m_typeBox = new QComboBox;
    m_typeBox->addItem(tr("Image"), int(wallpaper::Type::Image));
    if (m_options.allowSlideshow)
        m_typeBox->addItem(tr("Slideshow"), int(wallpaper::Type::Slideshow));
    m_typeBox->addItem(tr("Solid color"), int(wallpaper::Type::SolidColor));
    connect(m_typeBox, &QComboBox::currentIndexChanged, this, [this] {
        if (m_loading) return;
        m_config.type = wallpaper::Type(m_typeBox->currentData().toInt());
        if (m_config.type == wallpaper::Type::Slideshow)
            rescanFolder();
        syncUi();
        emitChanged(m_typeBox, true);
    });
    page->add_child(new settings_widget(tr("Background"), m_options.backgroundDescription, m_typeBox));

    m_imageControl = new QWidget;
    QHBoxLayout *imageLayout = new QHBoxLayout(m_imageControl);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(8);
    m_imageLabel = new QLabel;
    m_imageLabel->setObjectName("RowDescription");
    m_browsePhoto = new QPushButton(tr("Browse"));
    imageLayout->addWidget(m_imageLabel);
    imageLayout->addWidget(m_browsePhoto);
    connect(m_browsePhoto, &QPushButton::clicked, this, &WallpaperEditor::choosePhoto);
    page->add_child(new settings_widget(tr("Choose Photo"), "", m_imageControl));

    if (m_options.allowSlideshow) {
        m_folderControl = new QWidget;
        QHBoxLayout *folderLayout = new QHBoxLayout(m_folderControl);
        folderLayout->setContentsMargins(0, 0, 0, 0);
        folderLayout->setSpacing(8);
        m_folderLabel = new QLabel;
        m_folderLabel->setObjectName("RowDescription");
        m_browseFolder = new QPushButton(tr("Browse"));
        folderLayout->addWidget(m_folderLabel);
        folderLayout->addWidget(m_browseFolder);
        connect(m_browseFolder, &QPushButton::clicked, this, &WallpaperEditor::chooseFolder);
        page->add_child(new settings_widget(tr("Wallpaper Folder"), "", m_folderControl));
    }

    m_modeBox = new QComboBox;
    for (wallpaper::Mode mode : {wallpaper::Mode::Fill, wallpaper::Mode::Fit, wallpaper::Mode::Stretch, wallpaper::Mode::Center})
        m_modeBox->addItem(wallpaper::modeName(mode), int(mode));
    connect(m_modeBox, &QComboBox::currentIndexChanged, this, [this] {
        if (m_loading) return;
        m_config.mode = wallpaper::Mode(m_modeBox->currentData().toInt());
        syncUi();
        emitChanged(m_modeBox, true);
    });
    page->add_child(new settings_widget(tr("Display Mode"), "", m_modeBox));

    m_colorButton = new ColorButton;
    m_colorButton->setAlphaEnabled(false);
    connect(m_colorButton, &ColorButton::colorChanged, this, [this](const QColor &color) {
        if (m_loading) return;
        m_config.color = color;
        updatePreview();
        emitChanged(m_colorButton, true);
    });
    page->add_child(new settings_widget(tr("Background Color"), "", m_colorButton));

    if (m_options.allowSlideshow) {
        settings_widget_group *slideshowGroup = new settings_widget_group(tr("Slideshow"));
        page->add_child(slideshowGroup);

        m_intervalControl = new QWidget;
        QHBoxLayout *intervalLayout = new QHBoxLayout(m_intervalControl);
        intervalLayout->setContentsMargins(0, 0, 0, 0);
        intervalLayout->setSpacing(6);
        m_intervalSpin = new QSpinBox;
        m_intervalUnit = new QComboBox;
        m_intervalUnit->addItem(tr("Seconds"), int(Seconds));
        m_intervalUnit->addItem(tr("Minutes"), int(Minutes));
        m_intervalUnit->addItem(tr("Hours"), int(Hours));
        intervalLayout->addWidget(m_intervalSpin);
        intervalLayout->addWidget(m_intervalUnit);
        connect(m_intervalSpin, &QSpinBox::valueChanged, this, [this] {
            if (m_loading) return;
            applyIntervalFromUi(false);
        });
        connect(m_intervalUnit, &QComboBox::currentIndexChanged, this, [this] {
            if (m_loading) return;
            updateIntervalRange();
            applyIntervalFromUi(true);
        });
        slideshowGroup->add_child(new settings_widget(tr("Change every"), "", m_intervalControl));

        m_shuffleCheck = new QCheckBox;
        connect(m_shuffleCheck, &QCheckBox::toggled, this, [this](bool checked) {
            if (m_loading) return;
            m_config.shuffle = checked;
            emitChanged(m_shuffleCheck, true);
        });
        slideshowGroup->add_child(new settings_widget(tr("Shuffle order"),
            tr("Pick images from the folder in a random order"), m_shuffleCheck));
    }

    setConfig(wallpaper::Config{});
}

void WallpaperEditor::setConfig(const wallpaper::Config &config)
{
    m_loading = true;
    m_config = config;
    if (!m_options.allowSlideshow && m_config.type == wallpaper::Type::Slideshow)
        m_config.type = wallpaper::Type::Image;
    m_config.intervalSeconds = wallpaper::clampInterval(m_config.intervalSeconds);
    if (!m_config.color.isValid())
        m_config.color = Qt::black;
    m_config.color.setAlpha(255);

    {
        const QSignalBlocker b1(m_typeBox);
        const QSignalBlocker b2(m_modeBox);
        const QSignalBlocker b3(m_colorButton);
        m_typeBox->setCurrentIndex(qMax(0, m_typeBox->findData(int(m_config.type))));
        m_modeBox->setCurrentIndex(qMax(0, m_modeBox->findData(int(m_config.mode))));
        m_colorButton->setColor(m_config.color);
    }

    if (m_options.allowSlideshow) {
        setIntervalUi(m_config.intervalSeconds);
        const QSignalBlocker b(m_shuffleCheck);
        m_shuffleCheck->setChecked(m_config.shuffle);
        rescanFolder();
    }

    // Re-decode in case an in-place file (e.g. /var/lib/forest/login/wallpaper) was overwritten.
    m_previewPath.clear();
    m_previewSource = {};
    syncUi();
    m_loading = false;
}

void WallpaperEditor::choosePhoto()
{
    QString start;
    if (m_options.photoBrowseDir)
        start = m_options.photoBrowseDir(m_config.imagePath);
    if (start.isEmpty())
        start = m_config.imagePath.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                                             : QFileInfo(m_config.imagePath).path();
    const QString filter = tr("Images (%1)").arg(wallpaper::imageNameFilters().join(' '));
    const QString file = QFileDialog::getOpenFileName(m_browsePhoto->window(), tr("Choose Photo"), start, filter);
    if (file.isEmpty())
        return;
    m_config.imagePath = file;
    syncUi();
    emitChanged(m_imageControl, true);
}

void WallpaperEditor::chooseFolder()
{
    const QString start = m_config.directory.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : m_config.directory;
    const QString dir = QFileDialog::getExistingDirectory(m_browseFolder->window(), tr("Choose Wallpaper Folder"), start);
    if (dir.isEmpty())
        return;
    m_config.directory = dir;
    rescanFolder();
    syncUi();
    emitChanged(m_folderControl, true);
}

void WallpaperEditor::updateIntervalRange()
{
    const int mult = m_intervalUnit->currentData().toInt();
    const QSignalBlocker b(m_intervalSpin);
    if (mult == Seconds) {
        m_intervalSpin->setRange(wallpaper::minIntervalSeconds, 3600);
        m_intervalSpin->setSingleStep(5);
    } else if (mult == Minutes) {
        m_intervalSpin->setRange(1, 1440);
        m_intervalSpin->setSingleStep(1);
    } else {
        m_intervalSpin->setRange(1, 24);
        m_intervalSpin->setSingleStep(1);
    }
}

void WallpaperEditor::applyIntervalFromUi(bool immediate)
{
    const int mult = m_intervalUnit->currentData().toInt();
    m_config.intervalSeconds = wallpaper::clampInterval(m_intervalSpin->value() * mult);
    emitChanged(m_intervalControl, immediate);
}

void WallpaperEditor::setIntervalUi(int seconds)
{
    IntervalUnit unit = Minutes;
    int value = 15;
    if (seconds % Hours == 0) {
        unit = Hours;
        value = seconds / Hours;
    } else if (seconds % Minutes == 0) {
        unit = Minutes;
        value = seconds / Minutes;
    } else {
        unit = Seconds;
        value = seconds;
    }
    const QSignalBlocker b1(m_intervalUnit);
    const QSignalBlocker b2(m_intervalSpin);
    m_intervalUnit->setCurrentIndex(qMax(0, m_intervalUnit->findData(int(unit))));
    updateIntervalRange();
    m_intervalSpin->setValue(value);
}

void WallpaperEditor::rescanFolder()
{
    m_folderImages = wallpaper::listImagesInDirectory(m_config.directory);
}

void WallpaperEditor::syncUi()
{
    const bool isImage = m_config.type == wallpaper::Type::Image;
    const bool isSlideshow = m_options.allowSlideshow && m_config.type == wallpaper::Type::Slideshow;
    const bool isSolid = m_config.type == wallpaper::Type::SolidColor;
    const bool showColor = isSolid || ((isImage || isSlideshow)
        && (m_config.mode == wallpaper::Mode::Fit || m_config.mode == wallpaper::Mode::Center));

    setRowVisible(m_imageControl, isImage);
    setRowVisible(m_modeBox, isImage || isSlideshow);
    setRowVisible(m_colorButton, showColor);

    if (m_options.allowSlideshow) {
        setRowVisible(m_folderControl, isSlideshow);
        setRowVisible(m_intervalControl, isSlideshow);
        setRowVisible(m_shuffleCheck, isSlideshow);
    }

    if (isImage) {
        QString labelText;
        if (m_config.imagePath.isEmpty())
            labelText = tr("None");
        else if (m_options.formatImageName)
            labelText = m_options.formatImageName(m_config.imagePath);
        else
            labelText = QFileInfo(m_config.imagePath).fileName();
        m_imageLabel->setText(elided(m_imageLabel, labelText));
        m_imageLabel->setToolTip(m_config.imagePath);
    }

    if (isSlideshow) {
        QString text;
        if (m_config.directory.isEmpty()) {
            text = tr("No folder selected");
        } else {
            QString name = QFileInfo(m_config.directory).fileName();
            if (name.isEmpty())
                name = m_config.directory;
            const int count = m_folderImages.size();
            if (count == 0)
                text = tr("%1 (no images)").arg(name);
            else if (count == 1)
                text = tr("%1 (1 image)").arg(name);
            else
                text = tr("%1 (%2 images)").arg(name).arg(count);
        }
        m_folderLabel->setText(elided(m_folderLabel, text));
        m_folderLabel->setToolTip(m_config.directory);
    }

    updatePreview();
}

void WallpaperEditor::updatePreview()
{
    if (m_config.type == wallpaper::Type::SolidColor) {
        m_preview->setPixmap(QPixmap::fromImage(
            wallpaper::render({}, wallpaper::Mode::Fill, PreviewSize, m_config.color)));
        return;
    }

    QString sourcePath;
    if (m_config.type == wallpaper::Type::Image) {
        sourcePath = m_config.imagePath;
    } else if (m_config.type == wallpaper::Type::Slideshow) {
        for (const QString &candidate : std::as_const(m_folderImages)) {
            if (QImageReader(candidate).canRead()) {
                sourcePath = candidate;
                break;
            }
        }
    }

    if (sourcePath != m_previewPath) {
        m_previewPath = sourcePath;
        m_previewSource = sourcePath.isEmpty() ? QImage() : QImage(sourcePath);
    }

    m_preview->setPixmap(QPixmap::fromImage(
        wallpaper::render(m_previewSource, m_config.mode, PreviewSize, m_config.color)));
}

void WallpaperEditor::emitChanged(QWidget *control, bool immediate)
{
    if (!m_loading)
        emit changed(control, immediate);
}
