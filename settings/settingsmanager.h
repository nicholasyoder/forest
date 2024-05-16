#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QMainWindow>
#include <QSettings>

class SettingsManager : public QMainWindow
{
    Q_OBJECT

public:
    SettingsManager(QWidget *parent = nullptr);
    ~SettingsManager();

    void load_settings_ui();

};
#endif // SETTINGSMANAGER_H
