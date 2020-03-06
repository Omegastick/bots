#include <entt/entt.hpp>

#include "audio_system.h"
#include "audio/audio_engine.h"
#include "environment/components/audio_emitter.h"

namespace ai
{
void audio_system(entt::registry &registry, IAudioEngine &audio_engine)
{
    registry.view<AudioEmitter>().each([&](auto entity, auto &emitter) {
        audio_engine.play(audio_id_map[emitter.audio_id]);
        registry.assign<entt::tag<"should_destroy"_hs>>(entity);
    });
}
}