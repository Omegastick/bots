#include <entt/entt.hpp>
#include <glm/gtc/random.hpp>

#include "test_env.h"
#include "environment/systems/render_system.h"
#include "environment/systems/square_system.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
TestEnv::TestEnv()
{
    for (int i = 0; i < 100; ++i)
    {
        auto entity = registry.create();
        auto &rectangle = registry.assign<Rectangle>(entity);
        const glm::vec4 color{glm::linearRand(0.1f, 1.f),
                              glm::linearRand(0.1f, 1.f),
                              glm::linearRand(0.1f, 1.f),
                              1.f};
        rectangle.fill_color = set_alpha(color, 0.5f);
        rectangle.stroke_color = color;
        rectangle.stroke_width = 0.1f;
        rectangle.transform.rotate(glm::linearRand(0.f, 10.f));
        float scale = glm::linearRand(0.1f, 1.f);
        rectangle.transform.set_scale({scale, scale});
        rectangle.transform.set_position({glm::linearRand(-19.2f, 19.2f),
                                          glm::linearRand(-2.f, 12.f)});
    }
}

void TestEnv::draw(Renderer &renderer)
{
    render_system(registry, renderer);
}

void TestEnv::update(double delta_time)
{
    square_system(registry, delta_time);
}
}