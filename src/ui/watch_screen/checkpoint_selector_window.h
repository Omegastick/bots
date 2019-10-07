#pragma once

#include <memory>

namespace cpprl
{
class Policy;
}

namespace SingularityTrainer
{

class CheckpointSelectorWindow
{
  private:
    int selected_file;

  public:
    CheckpointSelectorWindow();

    std::unique_ptr<cpprl::Policy> update();
};
}