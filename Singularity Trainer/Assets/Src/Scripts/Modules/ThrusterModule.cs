using Actions;
using Scripts.Modules.Interfaces;
using SensorReadings;
using UnityEngine;

namespace Scripts.Modules
{
    public class ThrusterModule : Module, IThrustable
    {
        public float force = 10;
        public float thrustLength = 0.1f;

        private Rigidbody2D rigidBody;
        private float LastThrustTime { get; set; }

        protected override void Awake()
        {
            base.Awake();
            LastThrustTime = 0f;
            Actions.Add(new ThrustAction(this));
        }

        private void Start()
        {
            rigidBody = GetComponentInParent<Rigidbody2D>();
        }

        public void Thrust()
        {
            LastThrustTime = Time.time;
        }

        public void FixedUpdate()
        {
            if (Time.time - LastThrustTime < thrustLength)
            {
                var forceDirection = transform.TransformDirection(new Vector2(0, force));
                var forceOrigin = transform.TransformPoint(new Vector2(0, 0));
                rigidBody.AddForceAtPosition(forceDirection, forceOrigin);
            }
        }

        public override ISensorReading GetSensorReading()
        {
            return null;
        }
    }
}
