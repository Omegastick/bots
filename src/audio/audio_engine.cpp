#include <soloud.h>
#include <spdlog/spdlog.h>

#include "audio_engine.h"

namespace ai
{
AudioEngine::AudioEngine() : time(0)
{
    soloud = SoLoud::Soloud();
    if (soloud.init() != 0)
    {
        spdlog::error("Could not initialize audio engine");
    }
}

SoundHandle AudioEngine::play(AudioSource &audio_source)
{
    const int handle = soloud.playClocked(time, audio_source);
    return SoundHandle(*this, handle);
}

void AudioEngine::update(double delta_time)
{
    time += delta_time;
}
}