#pragma once

#include <memory>

namespace cpprl
{
class Policy;
}

namespace SingularityTrainer
{
class IO;

class CheckpointSelectorWindow
{
  private:
    int selected_file;
    IO *io;

  public:
    CheckpointSelectorWindow(IO &io);

    std::unique_ptr<cpprl::Policy> update();
};
}