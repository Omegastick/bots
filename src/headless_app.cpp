#include <chrono>
#include <fstream>
#include <memory>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "headless_app.h"
#include "training/trainer.h"

namespace ai
{
volatile sig_atomic_t stop;

void inthand(int /*signum*/)
{
    stop = 1;
}

HeadlessApp::HeadlessApp(TrainerFactory &trainer_factory)
    : trainer_factory(trainer_factory)
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

    std::ifstream file(args[1]);
    auto json = nlohmann::json::parse(file);
    TrainingProgram program(json);
    auto trainer = trainer_factory.make(program);

    auto last_evaluation_time = std::chrono::high_resolution_clock::now();
    while (!stop)
    {
        trainer->step_batch();
        if (std::chrono::high_resolution_clock::now() - last_evaluation_time > std::chrono::minutes(1))
        {
            trainer->evaluate();
            last_evaluation_time = std::chrono::high_resolution_clock::now();
        }
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