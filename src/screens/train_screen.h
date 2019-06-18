#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "screens/iscreen.h"
#include "training/training_program.h"
#include "training/trainers/trainer.h"

namespace SingularityTrainer
{
class Communicator;
class IO;
class PostProcLayer;
class Random;
class Renderer;
class ResourceManager;

class TrainScreen : public IScreen
{
  private:
    std::unique_ptr<PostProcLayer> crt_post_proc_layer;
    bool fast;
    IO &io;
    bool lightweight_rendering;
    glm::mat4 projection;
    ResourceManager &resource_manager;
    std::unique_ptr<ITrainer> trainer;

  public:
    TrainScreen(std::unique_ptr<ITrainer> trainer,
                IO &io,
                ResourceManager &resource_manager,
                Random &rng);

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(const double delta_time);
};

class TrainScreenFactory
{
  private:
    IO &io;
    ResourceManager &resource_manager;
    Random &rng;
    TrainerFactory &trainer_factory;

  public:
    TrainScreenFactory(IO &io,
                       ResourceManager &resource_manager,
                       Random &rng,
                       TrainerFactory &trainer_factory)
        : io(io),
          resource_manager(resource_manager),
          rng(rng),
          trainer_factory(trainer_factory) {}

    virtual std::shared_ptr<IScreen> make(TrainingProgram program)
    {
        auto trainer = trainer_factory.make(program);
        return std::make_unique<TrainScreen>(std::move(trainer),
                                             io,
                                             resource_manager,
                                             rng);
    }
};
}