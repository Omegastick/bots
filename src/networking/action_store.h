#pragma once

#include <map>
#include <vector>

namespace SingularityTrainer
{
class ActionStore
{
  private:
    std::map<int, std::vector<std::vector<int>>> actions;
    std::vector<int> actions_per_player;
    std::map<int, std::vector<bool>> action_received;

    void initialize_tick(int tick);

  public:
    ActionStore(std::vector<int> actions_per_player);

    void add_action(int tick, int player, const std::vector<int> &action);
    std::vector<std::vector<int>> get_actions(int tick);
    bool received_first_actions();
};
}