#pragma once

#include <memory>

namespace SingularityTrainer
{
class Checkpointer;
class IO;
class TrainingProgram;

class BrainWindow
{
  private:
    IO &io;

  public:
    BrainWindow(Checkpointer &checkpointer, IO &io);

    void update(TrainingProgram &program);
};
}