using System.Collections;
using System.Collections.Generic;
using UnityEngine;
namespace Bots
{
    public class BotController : MonoBehaviour
    {

        public MovementController movementController;
        public bool humanInput;

        private Rigidbody2D rigidBody;

        // Use this for initialization
        void Start()
        {
        }

        // Update is called once per frame
        void Update()
        {
            if (this.movementController != null)
            {
                if (this.humanInput) {
                    if (Input.GetKey(KeyCode.RightArrow))
                    {
                        this.movementController.MoveDirection = 0;
                        this.movementController.RotateDirection = -1;
                    }
                    else if (Input.GetKey(KeyCode.LeftArrow))
                    {
                        this.movementController.MoveDirection = 0;
                        this.movementController.RotateDirection = 1;
                    }
                    else if (Input.GetKey(KeyCode.UpArrow))
                    {
                        this.movementController.RotateDirection = 0;
                        this.movementController.MoveDirection = 1;
                    }
                    else if (Input.GetKey(KeyCode.DownArrow))
                    {
                        this.movementController.RotateDirection = 0;
                        this.movementController.MoveDirection = -1;
                    }
                    else
                    {
                        this.movementController.RotateDirection = 0;
                        this.movementController.MoveDirection = 0;
                    }
                }
            }
        }
    }
}

