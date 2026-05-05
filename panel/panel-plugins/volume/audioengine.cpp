// SPDX-License-Identifier: LGPL-3.0-or-later

#include "audioengine.h"

#include "audiodevice.h"

#include <QtDebug>

AudioEngine::AudioEngine(QObject *parent) :
    QObject(parent)
{
}


AudioEngine::~AudioEngine()
{
    qDeleteAll(m_sinks);
    m_sinks.clear();
}

void AudioEngine::mute(AudioDevice *device)
{
    setMute(device, true);
}

void AudioEngine::unmute(AudioDevice *device)
{
    setMute(device, false);
}

void AudioEngine::setIgnoreMaxVolume(bool ignore)
{
    Q_UNUSED(ignore)
}
