using Scripts;
using System;
using System.Collections.Generic;
using Training.Trainers;
using UnityEngine;

namespace Training.Environments
{
    public class TargetEnvironment : MonoBehaviour, IEnvironment
    {
        private Agent agent;
        private float reward;
        public ITrainer Trainer { get; set; }

        private float lastObservationTime;

        private void Awake()
        {
            lastObservationTime = Time.time;
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

        private void FixedUpdate()
        {
            if (Time.time - lastObservationTime > 0.1)
            {
                lastObservationTime = Time.time;
                Trainer.ObservationQueue.Enqueue(agent.GetObservation());
            }
        }
    }
}
