#pragma once

#include <string>

namespace SingularityTrainer
{
class Body;
class IO;

class SaveBodyWindow
{
  private:
    IO *io;
    std::string name;

  public:
    SaveBodyWindow(IO &io);

    bool update(Body &body);
};
}