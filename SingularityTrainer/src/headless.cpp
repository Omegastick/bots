#include <memory>
#include <unistd.h>
#include <signal.h>

#include <spdlog/spdlog.h>

#include "training/trainers/quick_trainer.h"
#include "communicator.h"
#include "random.h"
#include "requests.h"

using namespace SingularityTrainer;

volatile sig_atomic_t stop;

void inthand(int signum)
{
    stop = 1;
}

int main(int argc, const char *argv[])
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");

    signal(SIGINT, inthand);

    Communicator communicator("tcp://127.0.0.1:10201");
    Random rng(1);
    QuickTrainer trainer(&communicator, &rng, 7);
    trainer.begin_training();

    while (!stop)
    {
        trainer.step();
    }

    return 0;
}
