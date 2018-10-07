using SensorReadings;
using UnityEngine;
using Actions;

namespace Scripts.Modules
{
    public class GunModule: Module, Interfaces.IShootable
    {
        public GameObject projectile;
        public Transform projectileSpawnPoint;

        public override ISensorReading GetSensorReading()
        {
            return null;
        }

        public void Shoot()
        {
            GameObject projectileObject = Instantiate(projectile, projectileSpawnPoint.position, projectileSpawnPoint.rotation);
            projectileObject.GetComponent<Rigidbody2D>().AddRelativeForce(new Vector2(0, 1000));
        }

        public override void Awake()
        {
            base.Awake();
            Actions.Add(new ShootAction(this));
        }
    }
}
