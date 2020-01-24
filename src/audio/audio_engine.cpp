#include <soloud.h>
#include <spdlog/spdlog.h>

#include "audio_engine.h"
#include "misc/resource_manager.h"

namespace ai
{
AudioEngine::AudioEngine(ResourceManager &resource_manager)
    : resource_manager(resource_manager),
      time(0)
{
    soloud = SoLoud::Soloud();
    if (soloud.init() != 0 &&
        soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER) != 0)
    {
        spdlog::error("Could not initialize audio engine");
    }
}

SoundHandle AudioEngine::play(AudioSource &audio_source)
{
    const int handle = soloud.play(audio_source);
    return SoundHandle(*this, handle);
}

SoundHandle AudioEngine::play(const std::string &audio_source)
{
    return play(*resource_manager.audio_source_store.get(audio_source));
}

void AudioEngine::update(double delta_time)
{
    time += delta_time;
}
}