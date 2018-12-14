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
        public float moveTargetInterval = 60;
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
        private bool Done { get; set; }
        private List<bool> State { get; set; }
        private int StateCount { get; set; }
        private System.Random Rng { get; set; }

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
            Done = false;
            State = new List<bool> { false, true, false, true };
            Rng = new System.Random(this.GetInstanceID());
        }

        public void BeginTraining() { }

        public void ChangeReward(int agentNumber, float rewardDelta)
        {
            Reward += rewardDelta;
        }

        public Tuple<float, bool> GetReward(int agentNumber)
        {
            var tempReward = Reward;
            Reward = 0;
            var tempDone = Done;
            Done = false;
            return new Tuple<float, bool>(tempReward, tempDone);
        }

        public void Pause()
        {
            throw new NotImplementedException();
        }

        public void SendActions(int agentNumber, List<bool> actions)
        {
            Agent.Act(actions);
            // float reward = 0f;
            // for (int i = 0; i < actions.Count; i++)
            // {
            //     reward += actions[i] == State[i % 2] ? 1 : -1;
            // }
            // ChangeReward(agentNumber, reward);
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
                Trainer.ObservationQueue.Add(Agent.GetObservation());
                // for (int i = 0; i < State.Count; i++)
                // {
                //     if (Rng.NextDouble() < 0.05)
                //     {
                //         State[i] = !State[i];
                //     }
                // }
                // var observation = new LinearObservation();
                // observation.AgentNumber = 0;
                // observation.Environment = this;
                // var sensorReading = new LinearSensorReading();
                // sensorReading.Data = State.Select(x => x ? 1f : -1f).ToList();
                // observation.SensorReadings = new List<ISensorReading>() { sensorReading };
                // Trainer.ObservationQueue.Add(observation);
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
                Done = true;
            }
        }
    }
}
