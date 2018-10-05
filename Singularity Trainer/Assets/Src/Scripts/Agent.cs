using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using Observations;
using Scripts.Modules;
using Scripts.Actions;

namespace Scripts
{
    public class Agent: MonoBehaviour
    {
        public List<Module> Modules { get; protected set; }
        public List<BaseAction> Actions { get; protected set; }

        public void Act(List<int> actions)
        {
            throw new NotImplementedException();
        }

        public IObservation GetObservation()
        {
            throw new NotImplementedException();
        }
    }
}
