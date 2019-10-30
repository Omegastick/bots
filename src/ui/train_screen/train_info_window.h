#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace SingularityTrainer
{
class IO;

typedef std::pair<const std::vector<unsigned long long> &, const std::vector<float> &> IndexedData;

class IndexedDataStore
{
  private:
    std::vector<unsigned long long> indices;
    std::vector<float> values;

  public:
    void add_data(unsigned long long timestep, float value);
    const IndexedData get_data() const;
};

class TrainInfoWindow
{
  private:
    std::unordered_map<std::string, IndexedDataStore> data;
    IO &io;

  public:
    TrainInfoWindow(IO &io);

    void add_data(const std::string &label, unsigned long long timestep, float value);
    void update(unsigned long long timestep, unsigned int update);
};
}