#include <memory>

#include "communicator.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/trainers/quick_trainer.h"

namespace SingularityTrainer
{
QuickTrainer::QuickTrainer(ResourceManager &resource_manager, Communicator *communicator, int env_count)
    : communicator(communicator), waiting_for_server(false), env_count(env_count)
{
    for (int i = 0; i < env_count; ++i)
    {
        environments.push_back(std::make_unique<TargetEnv>(resource_manager, 460, 40, 1, 100));
    }
}

QuickTrainer::~QuickTrainer() {}

void QuickTrainer::begin_training()
{
    for (const auto &environment : environments)
    {
        environment->start_thread();
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
    hyperparams.num_minibatch = env_count;
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

void QuickTrainer::end_training()
{
    std::shared_ptr<EndSessionParam> end_session_param = std::make_shared<EndSessionParam>();
    end_session_param->session_id = 0;
    Request<EndSessionParam> end_session_request("end_session", end_session_param, 4);
    communicator->send_request<EndSessionParam>(end_session_request);
    communicator->get_response<EndSessionResult>();
}

void QuickTrainer::step() {}

void QuickTrainer::slow_step() {}

void QuickTrainer::save_model() {}
}