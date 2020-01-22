#pragma once

#include <soloud.h>

#include "audio/audio_source.h"
#include "audio/sound_handle.h"

#include "trompeloeil.hpp"

namespace ai
{
class ResourceManager;

enum class AudioDriver
{
    Null = 0,
    Auto = 1
};

class IAudioEngine
{
  public:
    virtual void init(AudioDriver driver) = 0;
    virtual SoundHandle play(AudioSource &audio_source) = 0;
    virtual SoundHandle play(const std::string &audio_source) = 0;
    virtual void update(double delta_time) = 0;
};

class AudioEngine : public IAudioEngine
{
  private:
    bool initialized;
    ResourceManager &resource_manager;
    SoLoud::Soloud soloud;
    double time;

  public:
    AudioEngine(ResourceManager &resource_manager);

    void init(AudioDriver driver) override;
    SoundHandle play(AudioSource &audio_source) override;
    SoundHandle play(const std::string &audio_source) override;
    void update(double delta_time) override;
};

class MockAudioEngine : public trompeloeil::mock_interface<IAudioEngine>
{
  public:
    IMPLEMENT_MOCK1(init);
    MAKE_MOCK1(play, SoundHandle(AudioSource &));
    MAKE_MOCK1(play, SoundHandle(const std::string &));
    MAKE_MOCK1(update, void(double));
};
}