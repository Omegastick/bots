using System;
using System.Collections.Generic;
using SensorReadings;
using Training.Environments;

namespace Observations
{
    public interface IObservation
    {
        IEnvironment Environment { get; set; }
        int AgentNumber { get; set; }
        List<ISensorReading> SensorReadings { get; set; }

        List<float[]> ToList();
    }
}
