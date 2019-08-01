#include <string>

#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <spdlog/spdlog.h>

#include "misc/module_factory.h"
#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/bodies/body.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/evaluators/elo_evaluator.h"
#include "training/saver.h"
#include "training/trainers/trainer.h"

namespace py = pybind11;
namespace di = boost::di;

using namespace SingularityTrainer;
PYBIND11_MODULE(singularity_trainer, m)
{
    m.doc() = "Singularity Trainer Python bindings";

    py::class_<Trainer>(m, "Trainer")
        .def("evaluate", &Trainer::evaluate)
        .def("save_model", [](Trainer &trainer, std::string directory) {
            return trainer.save_model(directory).string();
        })
        .def("step", &Trainer::step)
        .def("step_batch", &Trainer::step_batch);

    m.def("make_trainer", [](const std::string &program_json) {
        // Logging
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("%^[%T %7l] %v%$");

        auto json = nlohmann::json::parse(program_json);
        TrainingProgram program(json);

        const auto injector = di::make_injector(
            di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
            di::bind<int>.named(MaxSteps).to(600),
            di::bind<ISaver>.to<Saver>(),
            di::bind<std::string>.named(CheckpointDirectory).to("checkpoints"));
        auto trainer_factory = injector.create<TrainerFactory>();

        return trainer_factory.make(program);
    },
          "Make a Trainer");
}