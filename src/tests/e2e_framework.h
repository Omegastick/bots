namespace SingularityTrainer
{
class ScreenManager;

class Frame
{
  private:
    double delta_time;
    ScreenManager &screen_manager;

  public:
    Frame(double delta_time, ScreenManager &screen_manager);
    ~Frame();
    Frame(const Frame &) = delete;
    Frame &operator=(const Frame &) = delete;
};
}
