using SensorReadings;

namespace Scripts.Modules
{
    public class BaseModule : Module
    {
        public override Module RootModule
        {
            get
            {
                return this;
            }
        }

        public override ISensorReading GetSensorReading()
        {
            return null;
        }

        protected override void Awake()
        {
            base.Awake();
            parentModule = this;
        }
    }
}
