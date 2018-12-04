﻿using System;
using System.Collections.Generic;

namespace SensorReadings
{
    public class LinearSensorReading : ISensorReading
    {
        public List<float> Data { get; set; }

        public LinearSensorReading()
        {
            Data = new List<float>();
        }

        public List<float> ToList()
        {
            return Data;
        }
    }
}
