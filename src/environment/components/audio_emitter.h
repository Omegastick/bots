#pragma once

#include "misc/id_map.h"

namespace ai
{
inline IdMap audio_id_map;

struct AudioEmitter
{
    std::size_t audio_id = 0;
};
}