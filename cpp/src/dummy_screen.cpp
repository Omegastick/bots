#include <SFML/Graphics.hpp>

#include "communicator.h"
#include "requests.h"
#include "resource_manager.h"
#include "dummy_screen.h"

namespace SingularityTrainer
{
DummyScreen::DummyScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator)
{
    resource_manager->load_texture("arrow", "cpp/assets/images/Arrow.png");
    std::shared_ptr<sf::Texture> texture = resource_manager->texture_store.get("arrow");
    arrow = sf::Sprite(*texture);
    arrow.setRotation(45.f);
    arrow.setOrigin(arrow.getLocalBounds().width / 2.f, arrow.getLocalBounds().height / 2.f);
    arrow.setPosition(960.f, 540.f);

    // std::shared_ptr<BeginSessionParam> param = std::make_shared<BeginSessionParam>();
    // param->hyperparams.batch_size = 32;
    // param->hyperparams.gae = 0.9;
    // param->hyperparams.learning_rate = 0.0001;
    // param->model.inputs = 5;
    // param->model.outputs = 3;
    // param->contexts = 1;
    // param->auto_train = true;
    // param->session_id = 0;
    // param->training = true;
    // Request<BeginSessionParam> request("begin_session", param, 0);

    // communicator.send_request<BeginSessionParam>(request);
    // std::shared_ptr<Response<BeginSessionResult>> response = communicator.get_response<BeginSessionResult>();
    // std::cout << response->result << std::endl;
}
DummyScreen::~DummyScreen() {}

void DummyScreen::draw(sf::RenderTarget &render_target)
{
    render_target.draw(arrow);
}
void DummyScreen::update(float delta_time)
{
    arrow.rotate(delta_time * 5);
}
}