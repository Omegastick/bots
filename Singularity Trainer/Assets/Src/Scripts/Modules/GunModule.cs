using SensorReadings;
using UnityEngine;
using Actions;

namespace Scripts.Modules
{
    public class GunModule : Module, Interfaces.IShootable
    {
        public GameObject projectile;
        public Transform projectileSpawnPoint;
        public float cooldownTime = 0.2f;

        private float lastShotTime;
        private Rigidbody2D rigidBody;

        public override ISensorReading GetSensorReading()
        {
            return null;
        }

        public void Shoot()
        {
            if (Time.time - lastShotTime > cooldownTime)
            {
                lastShotTime = Time.time;
                GameObject projectileObject = Instantiate(projectile, projectileSpawnPoint.position, projectileSpawnPoint.rotation, transform);
                projectileObject.GetComponent<Rigidbody2D>().AddRelativeForce(new Vector2(0, 500));
                var forceDirection = transform.TransformDirection(new Vector2(0, -10));
                var forceOrigin = transform.TransformPoint(new Vector2(0, 0));
                rigidBody.AddForceAtPosition(forceDirection, forceOrigin);
            }
        }

        protected override void Awake()
        {
            base.Awake();
            Actions.Add(new ShootAction(this));
            rigidBody = GetComponentInParent<Rigidbody2D>();
        }

        private void Start()
        {
            lastShotTime = Time.time;
        }
    }
}
