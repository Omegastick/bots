using System;
using System.Collections.Generic;
using System.Linq;
using Observations;
using Scripts;
using SensorReadings;
using Training.Trainers;
using UnityEngine;

namespace Training.Environments
{
    public class TargetEnvironment : MonoBehaviour, IEnvironment
    {
        public ITrainer Trainer { get; set; }

        private Agent Agent { get; set; }
        private float Reward { get; set; }
        private Rigidbody2D AgentRigidBody { get; set; }
        private ValueDisplay ValueDisplay { get; set; }
        private List<Target> Targets { get; set; }
        private int Done { get; set; }
        private int CurrentStep { get; set; }
        private int MaxSteps { get; set; }

        private void Awake()
        {
            Targets = new List<Target>(GetComponentsInChildren<Target>());
            foreach (var target in Targets)
            {
                target.Environment = this;
            }
            Agent = GetComponentInChildren<Agent>();
            Agent.Environment = this;
            AgentRigidBody = Agent.GetComponent<Rigidbody2D>();
            ValueDisplay = GetComponentInChildren<ValueDisplay>();
            Done = 0;
            MaxSteps = 256;
            CurrentStep = 0;
        }

        public void BeginTraining() { }

        public void ChangeReward(int agentNumber, float rewardDelta)
        {
            Reward += rewardDelta;
        }

        public Tuple<float, int> GetReward(int agentNumber)
        {
            var tempReward = Reward;
            Reward = 0;
            var tempDone = Done;
            Done = 0;
            return new Tuple<float, int>(tempReward, tempDone);
        }

        public void Pause()
        {
            throw new NotImplementedException();
        }

        public void SendActions(int agentNumber, List<bool> actions)
        {
            Agent.Act(actions);
        }

        public void UnPause()
        {
            throw new NotImplementedException();
        }

        private void FixedUpdate()
        {
            CurrentStep++;
            if (CurrentStep % 4 == 0)
            {
                Trainer.ObservationQueue.Add(Agent.GetObservation());
            }
            if (CurrentStep >= MaxSteps)
            {
                foreach (var target in Targets)
                {
                    var targetRigidBody = target.GetComponent<Rigidbody2D>();
                    var newPosition = new Vector3(UnityEngine.Random.Range(-8f, 8f), UnityEngine.Random.Range(-8f, 8f));
                    var newWorldPosition = transform.TransformPoint(newPosition);
                    targetRigidBody.position = newWorldPosition;
                }
                var agentRigidBody = Agent.GetComponent<Rigidbody2D>();
                agentRigidBody.position = transform.TransformPoint(0, 0, 0);
                agentRigidBody.velocity = Vector2.zero;
                agentRigidBody.angularVelocity = 0;
                Done = 1;
                CurrentStep = 0;
            }
        }

        public void SetValue(int agentNumber, float value)
        {
            ValueDisplay.SetValue(value);
        }
    }
}
