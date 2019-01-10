#include <SFML/Graphics.hpp>

#include "communicator.h"
#include "requests.h"
#include "resource_manager.h"
#include "test_screen.h"

namespace SingularityTrainer
{
TestScreen::TestScreen(sf::RenderTarget &window, ResourceManager &resource_manager, Communicator &communicator)
{
    resource_manager.load_texture("arrow", "cpp/assets/images/Arrow.png");
    std::shared_ptr<sf::Texture> texture = resource_manager.texture_store.get("arrow");
    arrow = sf::Sprite(*texture);
    arrow.setRotation(45.0f);
    arrow.setOrigin(arrow.getLocalBounds().width / 2.0f, arrow.getLocalBounds().height / 2.0f);
    arrow.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

    std::shared_ptr<BeginSessionParam> param = std::make_shared<BeginSessionParam>();
    param->hyperparams.batch_size = 32;
    param->hyperparams.gae = 0.9;
    param->hyperparams.learning_rate = 0.0001;
    param->model.inputs = 5;
    param->model.outputs = 3;
    param->contexts = 1;
    param->auto_train = true;
    param->session_id = 0;
    param->training = true;
    Request<BeginSessionParam> request("begin_session", param, 0);

    communicator.send_request<BeginSessionParam>(request);
    std::shared_ptr<Response<BeginSessionResult>> response = communicator.get_response<BeginSessionResult>();
    std::cout << response->result << std::endl;
}
TestScreen::~TestScreen() {}

void TestScreen::draw(sf::RenderTarget &render_target)
{
    render_target.draw(arrow);
}
void TestScreen::update(float delta_time)
{
    arrow.rotate(delta_time * 5);
}
}