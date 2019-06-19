#pragma once

#include <memory>

namespace SingularityTrainer
{
class IO;
class TrainingProgram;

class BrainWindow
{
  private:
    IO &io;

  public:
    BrainWindow(IO &io);

    void update(TrainingProgram &program);
};
}