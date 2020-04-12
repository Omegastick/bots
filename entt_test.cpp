#include <future>
#include <vector>

#include <entt/entt.hpp>

struct TypeA
{
};
struct TypeB
{
};

void do_nothing(entt::registry &, entt::entity) {}

int main()
{
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 1000; i++)
    {
        futures.emplace_back(std::async(std::launch::async, [] {
            for (int j = 0; j < 1000; j++)
            {
                entt::registry registry;
                registry.on_destroy<TypeA>().connect<do_nothing>();
                for (int k = 0; k < 100000; k++)
                {
                    const auto entity = registry.create();
                    registry.emplace<TypeB>(entity);
                    registry.emplace<TypeA>(entity);
                }
            }
        }));
    }

    for (auto &future : futures)
    {
        future.get();
    }
}