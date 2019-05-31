#include <chrono>
#include <memory>
#include <signal.h>
#include <stdlib.h>

#include <spdlog/spdlog.h>

// #include "training/trainers/quick_trainer.h"
// #include "misc/random.h"

// using namespace SingularityTrainer;

volatile sig_atomic_t stop;

void inthand(int /*signum*/)
{
    stop = 1;
}

int main(int /*argc*/, const char * /*argv*/ [])
{
    // // Logging
    // spdlog::set_level(spdlog::level::debug);
    // spdlog::set_pattern("%^[%T %7l] %v%$");

    // signal(SIGINT, inthand);

    // Random rng(1);
    // QuickTrainer trainer(&rng, atoi(argv[1]));
    // trainer.begin_training();

    // auto last_save_time = std::chrono::steady_clock::now();

    // while (!stop)
    // {
    //     if (std::chrono::steady_clock::now() - last_save_time > std::chrono::minutes(10))
    //     {
    //         spdlog::info("Saving model");
    //         trainer.save_model();
    //         last_save_time = std::chrono::steady_clock::now();
    //     }
    //     trainer.step();
    // }

    return 0;
}
