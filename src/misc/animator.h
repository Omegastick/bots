#pragma once

#include <functional>
#include <vector>

namespace SingularityTrainer
{
struct Animation
{
    std::function<void(double)> step_function;
    double length;
    double elapsed_time = 0;
};

class Animator
{
  private:
    std::vector<Animation> animations;

  public:
    Animator();

    void add_animation(Animation &animation);
    void update(double delta_time);
};
}