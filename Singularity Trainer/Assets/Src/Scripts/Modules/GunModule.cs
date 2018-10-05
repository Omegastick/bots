using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SensorReadings;
using UnityEngine;

namespace Scripts.Modules
{
    public class GunModule: Module
    {
        public GameObject projectile;

        public override ISensorReading GetSensorReading()
        {
            return null;
        }

        public void Shoot()
        {
            throw new NotImplementedException();
        }
    }
}
