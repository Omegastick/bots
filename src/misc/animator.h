#pragma once

#include <functional>
#include <map>

namespace SingularityTrainer
{
struct Animation
{
    std::function<void(double)> step_function;
    double length;
    std::function<void()> finish_callback = [] {};
    double elapsed_time = 0;
};

class Animator
{
  private:
    std::map<unsigned long, Animation> animations;
    unsigned long current_id;

  public:
    Animator();

    unsigned long add_animation(Animation &&animation);
    void delete_animation(unsigned long id);
    void update(double delta_time);
};
}