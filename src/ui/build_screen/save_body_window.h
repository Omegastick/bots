#pragma once

#include <string>

namespace ai
{
class Animator;
class BuildEnv;
class IO;

class SaveBodyWindow
{
  private:
    unsigned long animation_id;
    Animator &animator;
    IO &io;
    std::string name;
    bool recently_saved;

  public:
    SaveBodyWindow(Animator &animator, IO &io);
    ~SaveBodyWindow();

    bool update(BuildEnv &build_env);
};
}