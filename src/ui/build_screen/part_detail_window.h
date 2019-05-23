#pragma once

namespace SingularityTrainer
{
class IModule;
class IO;

class PartDetailWindow
{
  private:
    IModule *selected_part;
    IO *io;

  public:
    PartDetailWindow(IO &io);

    void select_part(IModule *part);
    void update();
};
}