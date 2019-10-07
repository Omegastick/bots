#pragma once

#include <string>

namespace SingularityTrainer
{
class Body;

class SaveBodyWindow
{
  private:
    std::string name;

  public:
    SaveBodyWindow();

    bool update(Body &body);
};
}