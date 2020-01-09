#pragma once

#include "audio/audio_source.h"

namespace ai
{
class AudioEngine;

class SoundHandle
{
  private:
    AudioEngine &audio_engine;
    int handle;

  public:
    SoundHandle(AudioEngine &audio_engine, int handle);
};
}