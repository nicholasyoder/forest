// SPDX-License-Identifier: LGPL-3.0-or-later

#include "loginsettings.h"

#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>

#include "wallpapereditor.h"
#include "../../library/loginui/loginconfig.h"

LoginSettings::LoginSettings(){
    settings_item = new settings_page("appearance/lock-screen", "Login & Lock Wallpaper", "system-lock-screen", 15);
    settings_item->set_keywords({"wallpaper", "background", "greeter", "lock", "lockscreen",
                                 "image", "photo", "picture", "fill", "fit", "stretch",
                                 "center", "solid", "color", "colour"});
    connect(settings_item, &settings_category::opened, this, &LoginSettings::load);

    WallpaperEditor::Options opts;
    opts.allowSlideshow = false;
    opts.backgroundDescription = "Shown on the login screen and lock screen for all users. Changing it needs administrator permission.";
    opts.formatImageName = [](const QString &path) -> QString {
        if (path == QLatin1String(loginconfig::wallpaperFile))
            return "Custom image";
        if (path == QLatin1String(loginconfig::defaultWallpaper))
            return "Default";
        return QFileInfo(path).fileName();
    };
    opts.photoBrowseDir = [](const QString &path) -> QString {
        if (path.isEmpty() || path == QLatin1String(loginconfig::wallpaperFile)
            || path == QLatin1String(loginconfig::defaultWallpaper))
            return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        return QFileInfo(path).path();
    };
    editor = new WallpaperEditor(settings_item, opts, this);
    connect(editor, &WallpaperEditor::changed, this, &LoginSettings::update_apply);

    apply_button = new QPushButton("Apply (asks for password)");
    connect(apply_button, &QPushButton::clicked, this, &LoginSettings::apply);
    status = new QLabel;
    status->setWordWrap(true);
    status->hide();
    QFrame *apply_base = new QFrame;
    QHBoxLayout *apply_layout = new QHBoxLayout(apply_base);
    apply_layout->setContentsMargins(0, 0, 0, 0);
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
    installed_config = loginconfig::readWallpaper();
    status->hide();
    editor->setConfig(installed_config);
    update_apply();
}

bool LoginSettings::has_changes() const {
    const wallpaper::Config cfg = editor->config();
    if (cfg.type != installed_config.type)
        return true;
    if (cfg.type == wallpaper::Type::SolidColor)
        return cfg.color != installed_config.color;
    return cfg.imagePath != installed_config.imagePath
        || cfg.mode != installed_config.mode
        || cfg.color != installed_config.color;
}

void LoginSettings::update_apply(){
    apply_button->setEnabled(has_changes() && process->state() == QProcess::NotRunning);
}

void LoginSettings::apply(){
    const wallpaper::Config cfg = editor->config();
    const QString color = wallpaper::colorName(cfg.color);
    QStringList args;
    QByteArray image;

    if (cfg.type == wallpaper::Type::SolidColor) {
        args << "set-solid" << color;
    } else if (cfg.imagePath != installed_config.imagePath) {
        QFile file(cfg.imagePath);
        if (!file.open(QIODevice::ReadOnly)) {
            status->setText("Cannot read " + cfg.imagePath);
            status->show();
            return;
        }
        image = file.readAll();
        args << "set-wallpaper" << wallpaper::modeName(cfg.mode) << color;
    } else {
        args << "set-mode" << wallpaper::modeName(cfg.mode) << color;
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
    update_apply();
}
