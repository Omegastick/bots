#include "sound_handle.h"

namespace ai
{
SoundHandle::SoundHandle(IAudioEngine &audio_engine, int handle)
    : audio_engine(audio_engine),
      handle(handle) {}
}