#include <chrono>
#include <memory>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <spdlog/spdlog.h>

#include "headless_app.h"
#include "training/trainers/itrainer.h"

namespace SingularityTrainer
{
volatile sig_atomic_t stop;

void inthand(int /*signum*/)
{
    stop = 1;
}

HeadlessApp::HeadlessApp(Random &rng, ITrainer &trainer)
    : rng(rng), trainer(trainer)
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");

    signal(SIGINT, inthand);
}

int HeadlessApp::run(int argc, char *argv[])
{
    argh::parser args(argv);
    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    while (!stop)
    {
        trainer.step();
    }

    return 0;
}

int HeadlessApp::run_tests(int argc, char *argv[], const argh::parser &args)
{
    if (!args["--with-logs"])
    {
        spdlog::set_level(spdlog::level::off);
    }
    doctest::Context context;

    context.setOption("order-by", "name");

    context.applyCommandLine(argc, argv);

    return context.run();
}
}