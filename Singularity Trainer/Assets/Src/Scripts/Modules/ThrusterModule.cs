using Actions;
using Scripts.Modules.Interfaces;
using SensorReadings;
using UnityEngine;

namespace Scripts.Modules
{
    public class ThrusterModule : Module, IThrustable
    {
        public float force;

        private Rigidbody2D rigidBody;

        protected override void Awake()
        {
            base.Awake();
            Actions.Add(new ThrustAction(this));
        }

        private void Start()
        {
            rigidBody = GetComponentInParent<Rigidbody2D>();
        }

        public void Thrust()
        {
            var forceDirection = transform.TransformDirection(new Vector2(0, force));
            var forceOrigin = transform.TransformPoint(new Vector2(0, 0));
            rigidBody.AddForceAtPosition(forceDirection, forceOrigin);
        }

        public override ISensorReading GetSensorReading()
        {
            return null;
        }
    }
}
