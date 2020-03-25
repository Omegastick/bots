#pragma once

#include "graphics/colors.h"

namespace ai
{
class IO;

class ColorSchemeWindow
{
  private:
    IO *io;
    unsigned char selected_swatch;

  public:
    ColorSchemeWindow(IO &io);

    bool update(ColorScheme &color_scheme);
};
}