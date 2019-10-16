#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <tweeny.h>

#include "graphics/screens/animation_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/animator.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
AnimationTestScreen::AnimationTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    Animator &animator,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : animation_finished(true),
      animator(&animator),
      direction(Direction::Right),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>("base_module");
    sprite->transform.set_scale(glm::vec2(100, 100));
    sprite->transform.set_position(glm::vec2(-100, -100));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

AnimationTestScreen::~AnimationTestScreen() {}

void AnimationTestScreen::update(double delta_time)
{
    if (animation_finished)
    {
        std::shared_ptr<tweeny::tween<double>> tween;
        if (direction == Direction::Right)
        {
            tween = std::make_shared<tweeny::tween<double>>(tweeny::from(100.).to(1820.).during(1000).via(tweeny::easing::sinusoidalInOut));
        }
        else
        {
            tween = std::make_shared<tweeny::tween<double>>(tweeny::from(1820.).to(100.).during(1000).via(tweeny::easing::sinusoidalInOut));
        }
        Animation animation{
            [this, tween](float step_percentage) { sprite->transform.set_position({tween->step(step_percentage), 540}); },
            3,
            [this] {
                animation_finished = true;
                direction = direction == Direction::Right ? Direction::Left : Direction::Right;
            }};
        animator->add_animation(std::move(animation));
        animation_finished = false;
    }

    display_test_dialog("Animation test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->transform.rotate(1.f * delta_time);
}

void AnimationTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.draw(*sprite, projection);
    renderer.end();
}
}