using Scripts;
using System;
using System.Collections.Generic;
using Training.Trainers;
using UnityEngine;

namespace Training.Environments
{
    public class TargetEnvironment : MonoBehaviour, IEnvironment
    {
        public float moveTargetInterval = 10;
        public float agentResetInterval = 60;
        public ITrainer Trainer { get; set; }

        private Agent Agent { get; set; }
        private float Reward { get; set; }
        private float lastObservationTime;
        private Rigidbody2D AgentRigidBody { get; set; }
        private ValueDisplay ValueDisplay { get; set; }
        private float lastMoveTargetTime { get; set; }
        private float lastAgentResetTime { get; set; }
        private List<Target> Targets { get; set; }

        private void Awake()
        {
            lastObservationTime = Time.time;
            Targets = new List<Target>(GetComponentsInChildren<Target>());
            foreach (var target in Targets)
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

        private void Update()
        {
            if (Time.time - lastMoveTargetTime > moveTargetInterval)
            {
                lastMoveTargetTime = Time.time;
                foreach (var target in Targets)
                {
                    var rigidBody = target.GetComponent<Rigidbody2D>();
                    var newPosition = new Vector3(UnityEngine.Random.Range(-8f, 8f), UnityEngine.Random.Range(-8f, 8f));
                    var newWorldPosition = transform.TransformPoint(newPosition);
                    rigidBody.position = newWorldPosition;
                }
            }

            if (Time.time - lastAgentResetTime > agentResetInterval)
            {
                lastAgentResetTime = Time.time;

                var rigidBody = Agent.GetComponent<Rigidbody2D>();
                rigidBody.position = transform.TransformPoint(0, 0, 0);
                rigidBody.velocity = Vector2.zero;
                rigidBody.angularVelocity = 0;
            }
        }
    }
}
