#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <glm/mat4x4.hpp>

#include "graphics/post_processing/distortion_layer.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "training/trainer.h"
#include "ui/train_screen/train_info_window.h"

namespace SingularityTrainer
{
class Communicator;
class IO;
class PostProcLayer;
class Renderer;
class ResourceManager;
class ScreenManager;
class Trainer;

class TrainScreen : public IScreen
{
  private:
    bool batch_finished;
    std::thread batch_thread;
    std::unique_ptr<DistortionLayer> distortion_layer;
    bool fast;
    IO &io;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_eval_time;
    bool lightweight_rendering;
    glm::mat4 projection;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    std::unique_ptr<TrainInfoWindow> train_info_window;
    mutable std::mutex train_info_window_mutex;
    std::unique_ptr<Trainer> trainer;

  public:
    TrainScreen(std::unique_ptr<TrainInfoWindow> train_info_window,
                std::unique_ptr<Trainer> trainer,
                IO &io,
                ResourceManager &resource_manager,
                ScreenManager &screen_manager);
    ~TrainScreen();
    TrainScreen(const TrainScreen &) = delete;
    TrainScreen &operator=(const TrainScreen &) = delete;

    virtual void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);

    inline bool get_fast() const { return fast; }
    inline void set_fast(bool value) { fast = value; }
};

class TrainScreenFactory
{
  private:
    IO &io;
    ResourceManager &resource_manager;
    TrainerFactory &trainer_factory;
    ScreenManager &screen_manager;

  public:
    TrainScreenFactory(IO &io,
                       ResourceManager &resource_manager,
                       TrainerFactory &trainer_factory,
                       ScreenManager &screen_manager)
        : io(io),
          resource_manager(resource_manager),
          trainer_factory(trainer_factory),
          screen_manager(screen_manager) {}
    virtual ~TrainScreenFactory() {}

    virtual std::shared_ptr<IScreen> make(TrainingProgram program)
    {
        auto trainer = trainer_factory.make(program);
        return std::make_unique<TrainScreen>(std::make_unique<TrainInfoWindow>(io),
                                             std::move(trainer),
                                             io,
                                             resource_manager,
                                             screen_manager);
    }
};
}