#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace SingularityTrainer
{
class IAgent;
class Checkpointer;
class IO;

class ChooseAgentWindow
{
  private:
    Checkpointer &checkpointer;
    std::vector<std::filesystem::path> checkpoints;
    IO &io;
    int selected_file;

  public:
    ChooseAgentWindow(Checkpointer &checkpointer, IO &io);

    std::unique_ptr<IAgent> update();
};
}