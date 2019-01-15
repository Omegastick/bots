#include "test_screen/test_screen.h"

namespace SingularityTrainer
{
TestScreen::TestScreen(std::shared_ptr<ResourceManager> resource_manager, std::shared_ptr<Communicator> communicator, int env_count)
{
    this->communicator = communicator;
    resource_manager->load_texture("arrow", "cpp/assets/images/Arrow.png");
    for (int i = 0; i < env_count; ++i)
    {
        environments.push_back(std::make_unique<TestEnv>(resource_manager, 460, 40, 1, 100));
        environments.back()->start_thread();
    }
    std::vector<std::future<std::unique_ptr<StepInfo>>> observation_futures(env_count);
    observations.resize(env_count);
    for (int i = 0; i < environments.size(); ++i)
    {
        observation_futures[i] = environments[i]->reset();
    }
    for (int i = 0; i < environments.size(); ++i)
    {
        observations[i] = observation_futures[i].get()->observation;
    }

    // Init training session
    Model model{3, 4, true, true};
    HyperParams hyperparams;
    hyperparams.learning_rate = 0.0004;
    hyperparams.batch_size = 512;
    hyperparams.num_minibatch = 4;
    hyperparams.epochs = 3;
    hyperparams.discount_factor = 0.99;
    hyperparams.use_gae = true;
    hyperparams.gae = 0.95;
    hyperparams.critic_coef = 0.5;
    hyperparams.entropy_coef = 0.01;
    hyperparams.max_grad_norm = 0.5;
    hyperparams.clip_factor = 0.2;
    hyperparams.use_gpu = false;
    hyperparams.normalize_rewards = true;
    std::shared_ptr<BeginSessionParam> begin_session_param = std::make_shared<BeginSessionParam>();
    begin_session_param->session_id = 0;
    begin_session_param->model = std::move(model);
    begin_session_param->hyperparams = std::move(hyperparams);
    begin_session_param->contexts = env_count;
    begin_session_param->auto_train = true;
    begin_session_param->training = true;
    begin_session_param->session_id = 0;
    Request<BeginSessionParam> begin_session_request("begin_session", begin_session_param, 1);
    communicator->send_request<BeginSessionParam>(begin_session_request);
    communicator->get_response<BeginSessionResult>();
}

TestScreen::~TestScreen(){};

void TestScreen::update(float delta_time)
{
    // Get actions from training server
    std::shared_ptr<GetActionsParam> get_actions_param = std::make_shared<GetActionsParam>();
    get_actions_param->inputs = observations;
    get_actions_param->session_id = 0;
    Request<GetActionsParam> get_actions_request("get_actions", get_actions_param, 2);
    communicator->send_request(get_actions_request);
    std::vector<std::vector<int>> actions = communicator->get_response<GetActionsResult>()->result.actions;

    // Step environments
    std::vector<bool> dones;
    std::vector<float> rewards;
    std::vector<std::future<std::unique_ptr<StepInfo>>> step_info_futures(environments.size());
    for (int i = 0; i < environments.size(); ++i)
    {
        step_info_futures[i] = environments[i]->step(actions[i]);
    }
    for (int i = 0; i < environments.size(); ++i)
    {
        std::unique_ptr<StepInfo> step_info = step_info_futures[i].get();
        observations[i] = step_info->observation;
        dones.push_back(step_info->done);
        rewards.push_back(step_info->reward);
    }

    // Send result to training server
    std::shared_ptr<GiveRewardsParams> give_rewards_param = std::make_shared<GiveRewardsParams>();
    give_rewards_param->rewards = rewards;
    give_rewards_param->dones = dones;
    give_rewards_param->session_id = 0;
    Request<GiveRewardsParams> give_rewards_request("give_rewards", give_rewards_param, 3);
    communicator->send_request<GiveRewardsParams>(give_rewards_request);
    communicator->get_response<GiveRewardsResult>();
}

void TestScreen::draw(sf::RenderTarget &render_target)
{
    // for (auto &environment : environments)
    // {
    //     environment->draw(render_target);
    // }
    environments[0]->draw(render_target);
}
}