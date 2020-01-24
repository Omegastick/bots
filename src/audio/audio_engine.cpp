#include <soloud.h>
#include <spdlog/spdlog.h>

#include "audio_engine.h"
#include "misc/resource_manager.h"

namespace ai
{
AudioEngine::AudioEngine(ResourceManager &resource_manager)
    : disabled(false),
      resource_manager(resource_manager),
      time(0)
{
    soloud = SoLoud::Soloud();
    if (soloud.init() != 0)
    {
        spdlog::warn("Could not initialize audio engine");
        disabled = true;
    }
}

SoundHandle AudioEngine::play(AudioSource &audio_source)
{
    if (disabled)
    {
        return SoundHandle{*this, 0};
    }
    const int handle = soloud.play(audio_source);
    return SoundHandle(*this, handle);
}

SoundHandle AudioEngine::play(const std::string &audio_source)
{
    if (disabled)
    {
        return SoundHandle{*this, 0};
    }
    return play(*resource_manager.audio_source_store.get(audio_source));
}

void AudioEngine::update(double delta_time)
{
    if (disabled)
    {
        return;
    }
    time += delta_time;
}
}