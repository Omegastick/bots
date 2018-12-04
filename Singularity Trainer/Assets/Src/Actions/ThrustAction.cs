using Scripts.Modules;
using Scripts.Modules.Interfaces;

namespace Actions
{
    public class ThrustAction : Action
    {
        public ThrustAction(Module parentModule) : base(parentModule)
        {
            options = 2;
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
