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
