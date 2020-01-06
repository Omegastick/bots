#pragma once

#include <memory>

#include <argh.h>

namespace ai
{
class Random;
class TrainerFactory;

class HeadlessApp
{
  private:
    TrainerFactory &trainer_factory;

    int run_tests(int argc, char *argv[], const argh::parser &args);

  public:
    HeadlessApp(TrainerFactory &trainer_factory);

    int run(int argc, char *argv[]);
};
}
