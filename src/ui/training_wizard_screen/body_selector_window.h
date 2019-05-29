#pragma once

class b2World;

namespace SingularityTrainer
{
class Agent;
class IO;
class Random;

class BodySelectorWindow
{
  private:
    int last_selected_file;
    int selected_file;
    IO *io;

  public:
    BodySelectorWindow(IO &io);

    bool update(Random &rng, b2World &b2_world, Agent &agent);
};
}