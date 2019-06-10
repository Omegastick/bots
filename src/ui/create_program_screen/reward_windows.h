#pragma once

#include <memory>

namespace SingularityTrainer
{
class IO;
class RewardConfig;

class RewardWindows
{
  private:
    IO &io;

  public:
    RewardWindows(IO &io);

    void update(RewardConfig &reward_config);
};
}