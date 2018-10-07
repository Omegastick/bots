using System;
using System.Collections.Generic;
using UnityEngine;
using Observations;
using Scripts.Modules;

namespace Scripts
{
    public class Agent: MonoBehaviour
    {
        public List<Module> Modules { get; protected set; }
        public List<Actions.Action> Actions { get; protected set; }

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

        public void Act(List<int> actions)
        {
            for (int i = 0; i < Actions.Count; i++)
            {
                Actions[i].Act(actions[i]);
            }
        }

        public IObservation GetObservation()
        {
            throw new NotImplementedException();
        }
    }
}
