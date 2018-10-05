using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Training.Trainers
{
    public interface ITrainer
    {
        void BeginTraining();
        void EndTraining();
        void SaveModel(string path);
        void Step();
    }
}
