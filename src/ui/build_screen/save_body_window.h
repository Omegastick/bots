#pragma once

#include <string>

namespace SingularityTrainer
{
class Agent;
class IO;

class SaveBodyWindow
{
  private:
    IO *io;
    std::string name;

  public:
    SaveBodyWindow(IO &io);

    bool update(Agent &agent);
};
}