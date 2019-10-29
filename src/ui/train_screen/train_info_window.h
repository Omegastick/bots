#pragma once

#include <map>
#include <string>
#include <vector>
#include <unordered_map>

namespace SingularityTrainer
{
class IO;

class TrainInfoWindow
{
  private:
    std::unordered_map<std::string, std::map<unsigned long long, float>> data;
    IO &io;

  public:
    TrainInfoWindow(IO &io);

    void add_graph_data(const std::string &label, unsigned long long timestep, float value);
    void update(unsigned long long timestep, unsigned int update);
};
}