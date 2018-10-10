using Scripts.Modules;

namespace Actions
{
    public abstract class Action
    {
        public Module Module { get; private set; }
        public int options;

        public abstract void Act(int option);

        public Action(Module parentModule)
        {
            Module = parentModule;
        }
    }
}
