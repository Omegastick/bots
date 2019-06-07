#pragma once

#include <memory>

namespace SingularityTrainer
{
class IO;
class HyperParameters;

class AlgorithmWindow
{
  private:
    IO &io;

  public:
    AlgorithmWindow(IO &io);

    void update(HyperParameters &program);
};
}