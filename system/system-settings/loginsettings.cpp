// SPDX-License-Identifier: LGPL-3.0-or-later

#include "loginsettings.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>

#include <memory>

#include "miscutills.h"
#include "../../library/loginui/loginconfig.h"

namespace {
const QSize PreviewSize(400, 250);
}

LoginSettings::LoginSettings(){
    settings_item = new settings_page("appearance/lock-screen", "Lock Screen", "system-lock-screen", 15);
    settings_item->set_keywords({"wallpaper", "background", "greeter", "lock", "lockscreen", "image", "photo", "picture"});
    connect(settings_item, &settings_category::opened, this, &LoginSettings::load);

    QFrame *preview_base = new QFrame;
    preview_base->setObjectName("WallpaperPreviewBase");
    QHBoxLayout *preview_layout = new QHBoxLayout(preview_base);
    preview_layout->setContentsMargins(QMargins(0,0,0,0));
    preview_layout->setSpacing(0);
    preview_layout->addStretch(1);
    preview = new QLabel;
    preview->setObjectName("WallpaperPreviewImage");
    preview->setFixedSize(PreviewSize);
    preview_layout->addWidget(preview);
    preview_layout->addStretch(1);
    settings_item->add_child(new settings_widget("", "", preview_base, true));

    QPushButton *browse_button = new QPushButton("Browse");
    connect(browse_button, &QPushButton::clicked, this, &LoginSettings::choose_photo);
    settings_item->add_child(new settings_widget("Choose Photo",
        "Shown on the login screen and lock screen for all users. Changing it needs administrator permission.",
        browse_button));

    mode_box = new QComboBox;
    mode_box->addItems({"Fill", "Fit", "Stretch", "Tile", "Center"}); // miscutills WALLPAPER_MODE order
    connect(mode_box, &QComboBox::currentIndexChanged, this, &LoginSettings::update_ui);
    settings_item->add_child(new settings_widget("Display Mode", "", mode_box));

    apply_button = new QPushButton("Apply (asks for password)");
    connect(apply_button, &QPushButton::clicked, this, &LoginSettings::apply);
    status = new QLabel;
    status->setWordWrap(true);
    status->hide();
    QFrame *apply_base = new QFrame;
    QHBoxLayout *apply_layout = new QHBoxLayout(apply_base);
    apply_layout->setContentsMargins(QMargins(0,0,0,0));
    apply_layout->addWidget(status, 1);
    apply_layout->addWidget(apply_button);
    settings_item->add_child(new settings_widget("", "", apply_base, true));

    process = new QProcess(this);
    connect(process, &QProcess::finished, this, [this](int code){ apply_finished(code); });
    connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error){
        if (error == QProcess::FailedToStart) apply_finished(-1);
    });
}

void LoginSettings::load(){
    installed_custom = QFileInfo::exists(loginconfig::wallpaperFile);
    QSettings conf(loginconfig::confFile, QSettings::IniFormat);
    installed_mode = installed_custom ? conf.value(loginconfig::modeKey, 0).toInt() : 0;
    pending_image.clear();
    status->hide();
    {
        const QSignalBlocker blocker(mode_box);
        mode_box->setCurrentIndex(installed_mode);
    }
    update_ui();
}

QString LoginSettings::preview_source() const {
    if (!pending_image.isEmpty()) return pending_image;
    if (installed_custom) return loginconfig::wallpaperFile;
    return loginconfig::defaultWallpaper;
}

void LoginSettings::choose_photo(){
    QStringList patterns;
    foreach (QByteArray format, QImageReader::supportedImageFormats())
        patterns << "*." + QString::fromLatin1(format);
    QString start = pending_image.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                                            : QFileInfo(pending_image).path();
    QString file = QFileDialog::getOpenFileName(nullptr, "Choose Photo", start, "Images (" + patterns.join(' ') + ")");
    if (file.isEmpty()) return;
    pending_image = file;
    update_ui();
}

void LoginSettings::update_ui(){
    // Mode only applies to a custom image; the default is always shown Fill.
    bool custom = installed_custom || !pending_image.isEmpty();
    mode_box->setEnabled(custom);
    int mode = custom ? mode_box->currentIndex() : 0;

    std::unique_ptr<QImage> scaled(miscutills::get_wallpaper_scaled(preview_source(), WALLPAPER_MODE(mode), PreviewSize));
    preview->setPixmap(scaled ? QPixmap::fromImage(*scaled) : QPixmap());

    bool changed = !pending_image.isEmpty() || (installed_custom && mode != installed_mode);
    apply_button->setEnabled(changed && process->state() == QProcess::NotRunning);
}

void LoginSettings::apply(){
    int mode = mode_box->currentIndex();
    QStringList args;
    QByteArray image;
    if (!pending_image.isEmpty()) {
        QFile file(pending_image);
        if (!file.open(QIODevice::ReadOnly)) {
            status->setText("Cannot read " + pending_image);
            status->show();
            return;
        }
        image = file.readAll();
        args << "set-wallpaper" << QString::number(mode);
    } else {
        args << "set-mode" << QString::number(mode);
    }

    status->hide();
    apply_button->setEnabled(false);
    process->start("pkexec", QStringList{loginconfig::helperPath} + args);
    process->write(image);
    process->closeWriteChannel();
}

void LoginSettings::apply_finished(int exit_code){
    if (exit_code == 0) {
        load();
        return;
    }
    // pkexec: 126 = dialog dismissed or not authorized, 127 = authentication failed.
    if (exit_code == 126) status->setText("Cancelled.");
    else if (exit_code == 127) status->setText("Authentication failed.");
    else {
        QString error = QString::fromLocal8Bit(process->readAllStandardError()).trimmed();
        status->setText(error.isEmpty() ? "Could not apply the wallpaper." : error);
    }
    status->show();
    update_ui();
}
