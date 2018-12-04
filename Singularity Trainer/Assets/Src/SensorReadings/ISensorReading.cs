using System.Collections.Generic;

namespace SensorReadings
{
    public interface ISensorReading
    {
        List<float> ToList();
    }
}
