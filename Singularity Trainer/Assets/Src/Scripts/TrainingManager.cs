using UnityEngine;
using Training.Trainers;

namespace Scripts
{
    public class TrainingManager : MonoBehaviour
    {
        public QuickTrainer trainer;
        public float timeScale = 1;

        private void Start()
        {
            trainer.BeginTraining();
        }

        private void FixedUpdate()
        {
            trainer.Step();   
        }

        private void Update()
        {
            if (Input.GetKey(KeyCode.Space)) {
                timeScale = 1;
            } else {
                timeScale = 7;
            }
            Time.timeScale = timeScale;
        }
    }
}
