using System;
using System.Collections.Generic;
using Training.Trainers;

namespace Training.Environments
{
    public interface IEnvironment
    {
        ITrainer Trainer { get; set; }
        void BeginTraining();
        void SendActions(int agentNumber, List<bool> actions);
        Tuple<float, int> GetReward(int agentNumber);
        void ChangeReward(int agentNumber, float rewardDelta);
        void Pause();
        void UnPause();
        void SetValue(int agentNumber, float value);
    }
}
