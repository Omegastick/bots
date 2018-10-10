using System;
using System.Collections.Generic;
using UnityEngine;
using Scripts;

namespace Training.Environments
{
    public class TargetEnvironment : MonoBehaviour, IEnvironment
    {
        private Agent agent;
        private float reward;

        private void Awake()
        {
            Target[] targets = GetComponentsInChildren<Target>();
            foreach (var target in targets)
            {
                target.Environment = this;
            }
            agent = GetComponentInChildren<Agent>();
            agent.Environment = this;
        }

        public void BeginTraining()
        {
        }

        public void ChangeReward(int agentNumber, float rewardDelta)
        {
            reward += rewardDelta;
        }

        public float GetReward(int agentNumber)
        {
            var tempReward = reward;
            reward = 0;
            return tempReward;
        }

        public void Pause()
        {
            throw new NotImplementedException();
        }

        public void SendActions(int agentNumber, List<int> actions)
        {
            agent.Act(actions);
        }

        public void UnPause()
        {
            throw new NotImplementedException();
        }
    }
}
