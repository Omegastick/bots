using System;
using System.Collections.Generic;
using System.Linq;
using Training.Environments;
using SensorReadings;

namespace Observations
{
    public class LinearObservation : IObservation
    {
        public IEnvironment Environment { get; set; }
        public int AgentNumber { get; set; }
        public List<ISensorReading> SensorReadings { get; set; }

        public LinearObservation()
        {
            SensorReadings = new List<ISensorReading>();
        }

        public List<float[]> ToList()
        {
            List<float[]> dataList = new List<float[]>();
            foreach (var sensorReading in SensorReadings)
            {
                dataList.Add((sensorReading.ToArray() as float[]));
            }
            return dataList;
        }
    }
}
