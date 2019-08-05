#pragma once

#include <map>
#include <vector>

namespace SingularityTrainer
{
class ActionStore
{
  private:
    std::map<int, std::vector<std::vector<int>>> actions;
    std::map<int, std::vector<bool>> action_received;

  public:
    ActionStore(int num_players, std::vector<int> actions_per_player);

    void add_action(int tick, int player, std::vector<int> action);
    std::vector<std::vector<int>> get_actions(int tick);
};
}