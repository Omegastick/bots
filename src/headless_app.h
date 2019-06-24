#pragma once

#include <memory>

#include <argh.h>

namespace SingularityTrainer
{
class Random;
class TrainerFactory;

class HeadlessApp
{
  private:
    Random &rng;
    TrainerFactory &trainer_factory;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    HeadlessApp(Random &rng, TrainerFactory &trainer_factory);

    int run(int argc, char *argv[]);
};
}
