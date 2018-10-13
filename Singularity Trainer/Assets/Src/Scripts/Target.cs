using Training.Environments;
using UnityEngine;

namespace Scripts
{
    public class Target : MonoBehaviour
    {
        public IEnvironment Environment { get; set; }

        public void OnCollisionEnter2D(Collision2D collision)
        {
            if (Environment != null && collision.collider.gameObject.GetComponent<Bullet>() != null)
            {
                Environment.ChangeReward(0, 10);
            }
        }
    }
}
