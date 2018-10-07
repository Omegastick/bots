using SensorReadings;
using UnityEngine;
using Actions;

namespace Scripts.Modules
{
    public class GunModule: Module, Interfaces.IShootable
    {
        public GameObject projectile;
        public Transform projectileSpawnPoint;
        public float cooldownTime = 0.2f;

        private float lastShotTime;

        public override ISensorReading GetSensorReading()
        {
            return null;
        }

        public void Shoot()
        {
            if (Time.time - lastShotTime > cooldownTime)
            {
                lastShotTime = Time.time;
                GameObject projectileObject = Instantiate(projectile, projectileSpawnPoint.position, projectileSpawnPoint.rotation);
                projectileObject.GetComponent<Rigidbody2D>().AddRelativeForce(new Vector2(0, 1000));
            }
        }

        protected override void Awake()
        {
            base.Awake();
            Actions.Add(new ShootAction(this));
        }

        private void Start()
        {
            lastShotTime = Time.time;
        }
    }
}
