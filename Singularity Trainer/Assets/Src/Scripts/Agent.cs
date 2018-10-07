using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using Observations;
using Scripts.Modules;
using Actions;

namespace Scripts
{
    public class Agent: MonoBehaviour
    {
        public List<Module> Modules { get; protected set; }
        public List<BaseAction> Actions { get; protected set; }

        private void Awake()
        {
            Actions = new List<BaseAction>();
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
            Debug.Log("Received actions");
            for (int i = 0; i < Actions.Count; i++)
            {
                var debugString = "Sending action: " + actions[i];
                Debug.Log(debugString);
                Actions[i].Act(actions[i]);
            }
        }

        public IObservation GetObservation()
        {
            throw new NotImplementedException();
        }
    }
}
