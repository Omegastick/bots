using Scripts.Modules;
using Scripts.Modules.Interfaces;
using System.Collections.Generic;

namespace Actions
{
    public class ThrustAction : Action
    {
        public ThrustAction(Module parentModule) : base(parentModule)
        {
            options = 1;
        }

        public override void Act(List<bool> actions)
        {
            if (Module is IThrustable && actions[0] == true)
            {
                (Module as IThrustable).Thrust();
            }
        }
    }
}
