#pragma once

#include "graphics/colors.h"

namespace SingularityTrainer
{
class Body;
class IO;

class ColorSchemeWindow
{
  private:
    IO *io;
    unsigned char selected_swatch;

  public:
    ColorSchemeWindow(IO &io);

    void update(Body &body);
};
}