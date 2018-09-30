using UnityEngine;

namespace Bots
{
    public class MovementController : MonoBehaviour
    {
        float rotateDirection;
        public float RotateDirection
        {
            get
            {
                return rotateDirection;
            }
            set
            {
                rotateDirection = Mathf.Round(Mathf.Clamp(value, -1, 1));
            }
        }

        float moveDirection;
        public float MoveDirection
        {
            get
            {
                return moveDirection;
            }
            set
            {
                moveDirection = Mathf.Round(Mathf.Clamp(value, -1, 1));
            }
        }

        public float RotateSpeed = 1;
        public float MoveSpeed = 1;
        public float maxAngularVelocity = 7;

        private Rigidbody2D rigidBody;

        private void Start()
        {
            rigidBody = GetComponent<Rigidbody2D>();
        }

        private void FixedUpdate()
        {
            rigidBody.AddRelativeForce(new Vector2(0, MoveDirection * MoveSpeed));
            rigidBody.AddTorque(RotateDirection * RotateSpeed);
            if (rigidBody.angularVelocity < -maxAngularVelocity) { rigidBody.angularVelocity = -maxAngularVelocity; }
            if (rigidBody.angularVelocity > maxAngularVelocity) { rigidBody.angularVelocity = maxAngularVelocity; }
        }

        private void Update()
        {
           
        }
    }

}
