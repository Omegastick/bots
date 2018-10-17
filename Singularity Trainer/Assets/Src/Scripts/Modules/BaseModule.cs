﻿using SensorReadings;
using UnityEngine;

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

        private Rigidbody2D rigidBody { get; set; }

        public override ISensorReading GetSensorReading()
        {
            LinearSensorReading sensorReading = new LinearSensorReading();
            float xVelocity = rigidBody.velocity.x;
            float yVelocity = rigidBody.velocity.y;
            float angularVelocity = Mathf.Log(Mathf.Pow(rigidBody.angularVelocity + 1e-6f, 2));
            if (rigidBody.angularVelocity < 0)
            {
                angularVelocity *= -1;
            }
            sensorReading.Data.AddRange(new float[] { xVelocity, yVelocity, angularVelocity });
            return sensorReading;
        }

        protected override void Awake()
        {
            base.Awake();
            parentModule = this;
            rigidBody = GetComponentInParent<Rigidbody2D>();
        }
    }
}
