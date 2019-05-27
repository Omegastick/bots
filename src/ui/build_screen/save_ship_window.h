#pragma once

#include <string>

namespace SingularityTrainer
{
class Agent;
class IO;

class SaveShipWindow
{
  private:
    IO *io;
    std::string name;

  public:
    SaveShipWindow(IO &io);

    bool update(Agent &agent);
};
}