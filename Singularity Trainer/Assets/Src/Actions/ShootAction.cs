using Scripts.Modules.Interfaces;
using Scripts.Modules;

namespace Actions
{
    public class ShootAction : BaseAction
    {
        public ShootAction(Module parentModule) : base(parentModule)
        {
            options = 2;
        }

        public override void Act(int action)
        {
            if (Module is IShootable && action == 1)
            {
                Module.print("Shooting");
                (Module as IShootable).Shoot();
            }
        }
    }
}
