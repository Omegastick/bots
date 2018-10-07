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

        public override void Act(int action)
        {
            if (Module is IThrustable && action == 1)
            {
                (Module as IThrustable).Thrust();
            }
        }
    }
}
