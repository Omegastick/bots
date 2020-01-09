#pragma once

#include "audio/audio_source.h"

namespace ai
{
class IAudioEngine;

class SoundHandle
{
  private:
    IAudioEngine &audio_engine;
    int handle;

  public:
    SoundHandle(IAudioEngine &audio_engine, int handle);
};
}