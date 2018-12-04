using System.Collections.Generic;
using Observations;

namespace Training.Trainers
{
    public interface ITrainer
    {
        List<IObservation> ObservationQueue { get; set; }

        void BeginTraining();
        void EndTraining();
        void SaveModel(string path);
        void Step();
    }
}
