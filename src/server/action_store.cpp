#include <algorithm>
#include <map>
#include <vector>

#include <doctest.h>

#include "action_store.h"

namespace SingularityTrainer
{
ActionStore::ActionStore(std::vector<int> actions_per_player)
    : actions_per_player(actions_per_player)
{
    initialize_tick(0);
}

void ActionStore::initialize_tick(int tick)
{
    action_received[tick] = std::vector<bool>(actions_per_player.size(), false);

    for (const auto &num_actions : actions_per_player)
    {
        actions[tick].push_back(std::vector<int>(num_actions, 0));
    }
}

void ActionStore::add_action(int tick, int player, const std::vector<int> &action)
{
    // If tick hasn't been encountered yet, intialize it
    if (actions.find(tick) == actions.end())
    {
        initialize_tick(tick);
    }

    actions[tick][player] = action;
    action_received[tick][player] = true;
}

std::vector<std::vector<int>> ActionStore::get_actions(int tick)
{
    // If tick hasn't been encountered yet, intialize it
    if (actions.find(tick) == actions.end())
    {
        initialize_tick(tick);
    }

    for (unsigned int i = 0; i < actions_per_player.size(); ++i)
    {
        // If we haven't received an action for this step, use the latest
        // received action
        if (!action_received[tick][i])
        {
            for (auto iter = actions.rbegin(); iter != actions.rend(); ++iter)
            {
                if (action_received[iter->first][i])
                {
                    actions[tick][i] = iter->second[i];
                    break;
                }
            }
        }
    }

    return actions[tick];
}

bool ActionStore::received_first_actions()
{
    return std::all_of(action_received[0].begin(),
                       action_received[0].end(),
                       [](bool received) { return received; });
}

TEST_CASE("ActionStore")
{
    ActionStore action_store({2, 3});

    SUBCASE("Returns all 0s on first tick before any actions are added")
    {
        auto actions = action_store.get_actions(0);

        DOCTEST_CHECK(actions[0] == std::vector<int>{0, 0});
        DOCTEST_CHECK(actions[1] == std::vector<int>{0, 0, 0});
    }

    SUBCASE("Returns correct actions when actions have been set")
    {
        action_store.add_action(0, 0, {1, 0});
        action_store.add_action(0, 1, {1, 0, 1});

        auto actions = action_store.get_actions(0);

        DOCTEST_CHECK(actions[0] == std::vector<int>{1, 0});
        DOCTEST_CHECK(actions[1] == std::vector<int>{1, 0, 1});
    }

    SUBCASE("When actions are unavailable for a tick, uses latest action instead")
    {
        action_store.add_action(0, 0, {1, 0});
        action_store.add_action(0, 1, {1, 0, 1});
        action_store.add_action(1, 0, {1, 1});
        action_store.add_action(1, 1, {1, 1, 1});
        action_store.add_action(3, 1, {0, 1, 1});

        auto actions = action_store.get_actions(2);

        DOCTEST_CHECK(actions[0] == std::vector<int>{1, 1});
        DOCTEST_CHECK(actions[1] == std::vector<int>{0, 1, 1});
    }

    SUBCASE("received_first_actions()")
    {
        SUBCASE("Returns false when actions haven't been received")
        {
            DOCTEST_CHECK(!action_store.received_first_actions());
            action_store.add_action(0, 0, {1, 0});
            DOCTEST_CHECK(!action_store.received_first_actions());
        }

        SUBCASE("Returns true when actions have received")
        {
            action_store.add_action(0, 0, {1, 0});
            action_store.add_action(0, 1, {1, 0, 1});

            DOCTEST_CHECK(action_store.received_first_actions());
        }
    }
}
}