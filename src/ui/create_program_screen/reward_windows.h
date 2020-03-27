#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

namespace ai
{
class IO;
struct RewardConfig;

class RewardWindows
{
  private:
    IO &io;

  public:
    RewardWindows(IO &io);

    void update(glm::mat4 &projection, RewardConfig &reward_config);
};
}