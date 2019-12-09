#include <algorithm>
#include <future>
#include <vector>

#include <cpprl/cpprl.h>
#include <doctest.h>
#include <doctest/trompeloeil.hpp>
#include <torch/torch.h>

#include "multi_rollout_generator.h"
#include "graphics/renderers/renderer.h"
#include "training/rollout_generators/single_rollout_generator.h"

namespace SingularityTrainer
{
MultiRolloutGenerator::MultiRolloutGenerator(
    unsigned long num_steps,
    std::vector<std::unique_ptr<ISingleRolloutGenerator>> &&sub_generators)
    : batch_number(0),
      num_steps(num_steps),
      sub_generators(std::move(sub_generators))
{
}

void MultiRolloutGenerator::draw(Renderer &renderer, bool lightweight)
{
    sub_generators[0]->draw(renderer, lightweight);
}

cpprl::RolloutStorage MultiRolloutGenerator::generate()
{
    std::vector<std::future<cpprl::RolloutStorage>> storage_futures;
    for (auto &sub_generator : sub_generators)
    {
        storage_futures.emplace_back(std::async(std::launch::async, [&] {
            return sub_generator->generate(num_steps);
        }));
    }

    std::vector<cpprl::RolloutStorage> storages;
    std::transform(storage_futures.begin(), storage_futures.end(),
                   std::back_inserter(storages),
                   [](std::future<cpprl::RolloutStorage> &storage_future) {
                       return storage_future.get();
                   });

    std::vector<cpprl::RolloutStorage *> storage_ptrs;
    std::transform(storages.begin(), storages.end(), std::back_inserter(storage_ptrs),
                   [](cpprl::RolloutStorage &storage) { return &storage; });
    return {storage_ptrs, torch::kCPU};
}

TEST_CASE("MultiRolloutGenerator")
{
    std::vector<std::unique_ptr<ISingleRolloutGenerator>> sub_generators;
    std::vector<std::unique_ptr<trompeloeil::expectation>> expectations;
    for (int i = 0; i < 4; ++i)
    {
        auto sub_generator = std::make_unique<MockSingleRolloutGenerator>();
        expectations.push_back(NAMED_ALLOW_CALL(*sub_generator, generate(5))
                                   .LR_RETURN(cpprl::RolloutStorage(5,
                                                                    1,
                                                                    {23, 1},
                                                                    cpprl::ActionSpace{"Asd", {2}},
                                                                    0,
                                                                    torch::kCPU)));
        sub_generators.push_back(std::move(sub_generator));
    }
    MultiRolloutGenerator generator(5, std::move(sub_generators));

    SUBCASE("Generated rollouts are of the correct length")
    {
        const auto rollout = generator.generate();

        const auto length = rollout.get_actions().size(0);

        DOCTEST_CHECK(length == 5);
    }

    SUBCASE("Generated rollouts are of the correct width")
    {
        const auto rollout = generator.generate();

        const auto width = rollout.get_actions().size(1);

        DOCTEST_CHECK(width == 4);
    }
}
}