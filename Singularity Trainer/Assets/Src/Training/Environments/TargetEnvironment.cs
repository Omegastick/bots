using Scripts;
using System;
using System.Collections.Generic;
using Training.Trainers;
using UnityEngine;

namespace Training.Environments
{
    public class TargetEnvironment : MonoBehaviour, IEnvironment
    {
        private Agent Agent { get; set; }
        private float Reward { get; set; }
        public ITrainer Trainer { get; set; }

        private float lastObservationTime;
        private Rigidbody2D AgentRigidBody { get; set; }
        private ValueDisplay ValueDisplay { get; set; }

        private void Awake()
        {
            lastObservationTime = Time.time;
            Target[] targets = GetComponentsInChildren<Target>();
            foreach (var target in targets)
            {
                target.Environment = this;
            }
            Agent = GetComponentInChildren<Agent>();
            Agent.Environment = this;
            AgentRigidBody = Agent.GetComponent<Rigidbody2D>();
            ValueDisplay = GetComponentInChildren<ValueDisplay>();
        }

        public void BeginTraining()
        {
        }

        public void ChangeReward(int agentNumber, float rewardDelta)
        {
            Reward += rewardDelta;
        }

        public float GetReward(int agentNumber)
        {
            if (Mathf.Abs(AgentRigidBody.angularVelocity) > 60)
            {
                Debug.Log(AgentRigidBody.angularVelocity);
                Reward -= Mathf.Abs(AgentRigidBody.angularVelocity) / 360;
            }

            var tempReward = Reward;
            Reward = 0;
            return tempReward;
        }

        public void Pause()
        {
            throw new NotImplementedException();
        }

        public void SendActions(int agentNumber, List<int> actions)
        {
            Agent.Act(actions);
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
                Trainer.ObservationQueue.Enqueue(Agent.GetObservation());
            }
        }

        public void SetValue(int agentNumber, float value)
        {
            ValueDisplay.SetValue(value);
        }
    }
}
