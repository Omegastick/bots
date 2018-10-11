using UnityEngine;
using SensorReadings;

namespace Scripts.Modules
{
    public class LaserSensorModule : Module
    {
        public int rays = 21;
        public float spread = 180f;
        public float range = 10f;

        private new PolygonCollider2D collider;

        protected override void Awake()
        {
            base.Awake();
            collider = GetComponent<PolygonCollider2D>();
        }

        public override ISensorReading GetSensorReading()
        {
            collider.enabled = false;
            var sensorReading = new LinearSensorReading();
            for (int i = 0; i < rays; i++)
            {
                Vector2 direction = Quaternion.Euler(0, 0, (i * (spread / (rays - 1))) - (spread / 2)) * Vector2.up;
                RaycastHit2D hit = Physics2D.Raycast(transform.position, transform.rotation * direction, range);
                if (hit != false)
                {
                    sensorReading.Data.Add(hit.distance / range);
                    Debug.DrawRay(transform.position, transform.rotation * direction * hit.distance);
                }
                else
                {
                    sensorReading.Data.Add(1f);
                }
            }
            collider.enabled = true;
            return sensorReading;
        }
    }
}
