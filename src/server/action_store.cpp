#include <map>
#include <vector>

#include <doctest.h>

#include "action_store.h"

namespace SingularityTrainer
{
ActionStore::ActionStore(int num_players, std::vector<int> actions_per_player)
{
}

void ActionStore::add_action(int tick, int player, std::vector<int> action) {}
std::vector<std::vector<int>> ActionStore::get_actions(int tick) {}

TEST_CASE("ActionStore")
{
    ActionStore action_store(2, {2, 3});

    SUBCASE("Throws if less than one player")
    {
        DOCTEST_CHECK_THROWS(ActionStore action_store(0, {}));
        DOCTEST_CHECK_THROWS(ActionStore action_store(-1, {}));
    }

    SUBCASE("Throws if number of players doesn't match number of actions per player")
    {
        DOCTEST_CHECK_THROWS(ActionStore action_store(1, {1, 2}));
        DOCTEST_CHECK_THROWS(ActionStore action_store(5, {2, 3, 4}));
    }

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
}
}