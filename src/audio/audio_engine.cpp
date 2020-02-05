#include <soloud.h>
#include <spdlog/spdlog.h>

#include "audio_engine.h"
#include "misc/resource_manager.h"

namespace ai
{
AudioEngine::AudioEngine(ResourceManager &resource_manager)
    : initialized(false),
      resource_manager(resource_manager),
      time(0)
{
}

void AudioEngine::init(AudioDriver driver)
{
    soloud = SoLoud::Soloud();
    const auto soloud_driver = driver == AudioDriver::Null
                                   ? SoLoud::Soloud::NULLDRIVER
                                   : SoLoud::Soloud::AUTO;

    if (soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF, soloud_driver) != 0)
    {
        spdlog::error("Could not initialize audio engine");
        return;
    }

    initialized = true;
    return;
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