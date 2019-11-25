#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "texture_store_test_screen.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/test_utils.h"
#include "misc/module_texture_store.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/transform.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
TextureStoreTestScreen::TextureStoreTestScreen(ModuleTextureStore &module_texture_store,
                                               ScreenManager &screen_manager,
                                               ResourceManager &resource_manager,
                                               std::vector<std::shared_ptr<IScreen>> &screens,
                                               std::vector<std::string> &screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(-19.2f, 19.2f, -10.8f, 10.8f)),
      module_texture_store(module_texture_store)
{
    module_texture_store.get("base_module");
    auto &texture = module_texture_store.get("gun_module");
    resource_manager.texture_store.add_raw("base_tex", &texture);
}

void TextureStoreTestScreen::update(double delta_time)
{
    display_test_dialog("Texture store test", screens, screen_names, delta_time, screen_manager);
}

void TextureStoreTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    Transform transform;
    transform.set_scale({1, 1});
    renderer.draw(Sprite{{1, 1, 1, 1}, "base_tex", Transform()});
}
}