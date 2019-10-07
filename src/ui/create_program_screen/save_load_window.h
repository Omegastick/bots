#pragma once

#include <memory>

namespace SingularityTrainer
{
class IO;
struct TrainingProgram;

class SaveLoadWindow
{
  private:
    IO &io;
    std::string name;

  public:
    SaveLoadWindow(IO &io);

    bool update(TrainingProgram &program);
};
}