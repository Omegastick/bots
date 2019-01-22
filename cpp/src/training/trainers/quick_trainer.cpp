#include "training/trainers/quick_trainer.h"

namespace SingularityTrainer
{
QuickTrainer::QuickTrainer(Communicator &communicator) : communicator(communicator) {}
QuickTrainer::~QuickTrainer() {}

void QuickTrainer::begin_training() {}

void QuickTrainer::end_training() {}

void QuickTrainer::step() {}

void QuickTrainer::save_model() {}
}