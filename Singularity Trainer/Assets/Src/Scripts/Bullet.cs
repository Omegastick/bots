using UnityEngine;

namespace Scripts
{
    public class Bullet : MonoBehaviour
    {
        public float lifeTime = 2;

        private float spawnTime;

        private void Start()
        {
            spawnTime = Time.time;
        }

        private void Update()
        {
            if (Time.time - spawnTime > lifeTime)
            {
                Destroy(gameObject);
            }
        }
    }
}
