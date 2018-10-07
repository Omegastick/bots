using UnityEngine;
using Scripts.Modules;

namespace Actions
{
    public abstract class BaseAction
    {
        public Module Module { get; private set; }
        public int options;

        public abstract void Act(int option);

        public BaseAction(Module parentModule)
        {
            Module = parentModule;
        }
    }
}
