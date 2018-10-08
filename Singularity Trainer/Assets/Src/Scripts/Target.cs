using Training.Environments;
using UnityEngine;

namespace Scripts
{
    public class Target : MonoBehaviour
    {
        public IEnvironment Environment;

        public void OnCollisionEnter2D(Collision2D collision)
        {
            if (Environment != null && collision.otherCollider.gameObject.GetComponent<Bullet>() != null)
            {
                Environment.ChangeReward(0, 1);
            }
        }
    }
}
