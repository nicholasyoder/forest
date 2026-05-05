// SPDX-License-Identifier: LGPL-3.0-or-later

#include "alsadevice.h"

AlsaDevice::AlsaDevice(AudioDeviceType t, AudioEngine *engine, QObject *parent) :
    AudioDevice(t, engine, parent),
    m_mixer(nullptr),
    m_elem(nullptr)
{

}


void AlsaDevice::setMixer(snd_mixer_t *mixer)
{
    if (m_mixer == mixer)
        return;

    m_mixer = mixer;
    emit mixerChanged();
}

void AlsaDevice::setElement(snd_mixer_elem_t *elem)
{
    if (m_elem == elem)
        return;

    m_elem = elem;
    emit elementChanged();
}

void AlsaDevice::setCardName(const QString &cardName)
{
    if (m_cardName == cardName)
        return;

    m_cardName = cardName;
    emit cardNameChanged();
}
