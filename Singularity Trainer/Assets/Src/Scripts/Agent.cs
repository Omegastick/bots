using System;
using System.Collections.Generic;
using UnityEngine;
using Observations;
using Scripts.Modules;
using Training.Environments;

namespace Scripts
{
    public class Agent: MonoBehaviour
    {
        public List<Module> Modules { get; protected set; }
        public List<Actions.Action> Actions { get; protected set; }
        public IEnvironment Environment { get; set; }

        private void Awake()
        {
            Actions = new List<Actions.Action>();
            BaseModule baseModule = GetComponentInChildren<BaseModule>();
            Modules = baseModule.GetChildren();
            foreach (Module module in Modules)
            {
                if (module.Actions != null)
                {
                    Actions.AddRange(module.Actions);
                }
            }
        }

        public void Act(List<int> inputs)
        {
            int inputCounter = 0;
            foreach (var action in Actions)
            {
                int actionInputCount = action.options;
                var actionInputs = inputs.Skip(inputCounter).Take(actionInputCount);
                inputCounter += actionInputCount;
                Actions[i].Act(actionInputs);
            }
        }

        public IObservation GetObservation()
        {
            var observation = new LinearObservation();
            observation.Environment = Environment;
            foreach (var module in Modules)
            {
                var sensorReading = module.GetSensorReading();
                if (sensorReading != null)
                {
                    observation.SensorReadings.Add(sensorReading);
                }
            }
            return observation;
        }
    }
}
