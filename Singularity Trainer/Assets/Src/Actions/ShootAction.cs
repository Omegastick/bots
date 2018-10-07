using Scripts.Modules.Interfaces;
using Scripts.Modules;

namespace Actions
{
    public class ShootAction : Action
    {
        public ShootAction(Module parentModule) : base(parentModule)
        {
            options = 2;
        }

        public override void Act(int action)
        {
            if (Module is IShootable && action == 1)
            {
                (Module as IShootable).Shoot();
            }
        }
    }
}
