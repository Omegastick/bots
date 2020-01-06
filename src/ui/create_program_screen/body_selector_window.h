#pragma once

#include <memory>

#include <nlohmann/json_fwd.hpp>

namespace ai
{
class IO;

class BodySelectorWindow
{
  private:
    IO &io;
    int last_selected_file;
    int selected_file;

  public:
    BodySelectorWindow(IO &io);

    nlohmann::json update();
};
}