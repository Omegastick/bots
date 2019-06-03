#pragma once

#include <argh.h>

namespace SingularityTrainer
{
class Random;
class ITrainer;

class HeadlessApp
{
  private:
    Random &rng;
    ITrainer &trainer;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    HeadlessApp(Random &rng, ITrainer &trainer);

    int run(int argc, char *argv[]);
};
}
