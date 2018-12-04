using Scripts.Modules;
using System.Collections.Generic;

namespace Actions
{
    public abstract class Action
    {
        public Module Module { get; private set; }
        public int options;

        public abstract void Act(List<bool> options);

        public Action(Module parentModule)
        {
            Module = parentModule;
        }
    }
}
