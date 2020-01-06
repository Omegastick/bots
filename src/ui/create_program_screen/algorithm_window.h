#pragma once

#include <memory>

namespace ai
{
class IO;
struct HyperParameters;

class AlgorithmWindow
{
  private:
    IO &io;

  public:
    AlgorithmWindow(IO &io);

    void update(HyperParameters &hyperparams);
};
}