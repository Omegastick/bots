#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

namespace SingularityTrainer
{
class IEnvironment;
class IO;
struct RewardConfig;

class RewardWindows
{
  private:
    IO &io;

  public:
    RewardWindows(IO &io);

    void update(IEnvironment &environment, glm::mat4 &projection, RewardConfig &reward_config);
};
}