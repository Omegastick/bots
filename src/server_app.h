#pragma once

#include <memory>

#include <argh.h>

namespace SingularityTrainer
{
class ServerApp
{
  private:
    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    ServerApp();

    int run(int argc, char *argv[]);
};
}
