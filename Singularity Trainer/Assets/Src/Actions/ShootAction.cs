using Scripts.Modules.Interfaces;
using Scripts.Modules;

namespace Actions
{
    public class ShootAction : Action
    {
        public ShootAction(Module parentModule) : base(parentModule)
        {
            options = 1;
        }

        public override void Act(List<bool> actions)
        {
            if (Module is IShootable && actions[0] == true)
            {
                (Module as IShootable).Shoot();
            }
        }
    }
}
