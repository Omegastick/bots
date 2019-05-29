#pragma once

#include <memory>

class b2World;

namespace SingularityTrainer
{
class Agent;
class IO;
class Random;

class BodySelectorWindow
{
  private:
    int selected_file;
    IO *io;

  public:
    BodySelectorWindow(IO &io);

    std::unique_ptr<Agent> update(Random &rng, b2World &b2_world);
};
}