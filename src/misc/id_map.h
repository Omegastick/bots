#pragma once

#include <shared_mutex>
#include <string>

#include "third_party/bimap.hpp"

namespace ai
{
class IdMap
{
  private:
    stde::unordered_bimap<std::size_t, std::string> map;
    std::shared_mutex mutex;

  public:
    std::size_t operator[](const std::string &audio_name);
    const std::string operator[](const std::size_t audio_id);
};
}