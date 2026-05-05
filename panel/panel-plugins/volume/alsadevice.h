// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ALSADEVICE_H
#define ALSADEVICE_H

#include "audiodevice.h"

#include <alsa/asoundlib.h>


#include <QtCore/QObject>
#include <QtCore/QString>

/**
 * @brief The AlsaDevice class
 */
class AlsaDevice : public AudioDevice
{
    Q_OBJECT

public:
    AlsaDevice(AudioDeviceType t, AudioEngine *engine, QObject *parent = 0);

    snd_mixer_t *mixer() const { return m_mixer; }
    snd_mixer_elem_t *element() const { return m_elem; }
    const QString &cardName() const { return m_cardName; }

    void setMixer(snd_mixer_t *mixer);
    void setElement(snd_mixer_elem_t *elem);
    void setCardName(const QString &cardName);

signals:
    void mixerChanged();
    void elementChanged();
    void cardNameChanged();

private:
    snd_mixer_t *m_mixer;
    snd_mixer_elem_t *m_elem;
    QString m_cardName;
};

#endif // ALSADEVICE_H
