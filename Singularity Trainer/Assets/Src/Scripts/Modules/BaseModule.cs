using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SensorReadings;

namespace Scripts.Modules
{
    public class BaseModule : Module
    {
        public override Module ParentModule
        {
            get
            {
                return null;
            }
        }

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
    }
}
