// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ALSAENGINE_H
#define ALSAENGINE_H

#include "audioengine.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>


#include <alsa/asoundlib.h>


class AlsaDevice;
class QSocketNotifier;

/**
 * @brief The AlsaEngine class
 */
class AlsaEngine : public AudioEngine
{
    Q_OBJECT

public:
    AlsaEngine(QObject *parent = 0);
    ~AlsaEngine();
    static AlsaEngine *instance();

    virtual const QString backendName() const { return QLatin1String("Alsa"); }

    int volumeMax(AudioDevice *device) const;
    AlsaDevice *getDeviceByAlsaElem(snd_mixer_elem_t *elem) const;

public slots:
    void commitDeviceVolume(AudioDevice *device);
    void setMute(AudioDevice *device, bool state);
    void updateDevice(AlsaDevice *device);

private slots:
    void driveAlsaEventHandling(int fd);

private:
    void discoverDevices();
    QMap<int, snd_mixer_t *> m_mixerMap;
    static AlsaEngine *m_instance;
};

#endif // ALSAENGINE_H
