#pragma once

#include <soloud.h>

#include "audio/audio_source.h"
#include "audio/sound_handle.h"

namespace ai
{

class AudioEngine
{
  private:
    SoLoud::Soloud soloud;
    double time;

  public:
    AudioEngine();

    SoundHandle play(AudioSource &audio_source);
    void update(double delta_time);
};
}