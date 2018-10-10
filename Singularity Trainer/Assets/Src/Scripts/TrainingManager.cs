using UnityEngine;
using Training.Trainers;

namespace Scripts
{
    public class TrainingManager : MonoBehaviour
    {
        public QuickTrainer trainer;

        private void Start()
        {
            trainer.BeginTraining();
        }

        private void FixedUpdate()
        {
            trainer.Step();   
        }
    }
}
