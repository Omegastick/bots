using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Training.Environments
{
    public interface IEnvironment
    {
        void BeginTraining();
        void SendActions(int agentNumber, List<int> actions);
        float GetReward(int agentNumber);
        void ChangeReward(int agentNumber, float rewardDelta);
        void Pause();
        void UnPause();
    }
}
