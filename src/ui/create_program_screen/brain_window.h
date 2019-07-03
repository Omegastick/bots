#pragma once

#include <filesystem>
#include <memory>

namespace SingularityTrainer
{
class Checkpointer;
class IO;
class TrainingProgram;

class BrainWindow
{
  private:
    Checkpointer &checkpointer;
    std::vector<std::filesystem::path> checkpoints;
    IO &io;
    int last_selected_file;
    int selected_file;

  public:
    BrainWindow(Checkpointer &checkpointer, IO &io);

    void update(TrainingProgram &program);
};
}