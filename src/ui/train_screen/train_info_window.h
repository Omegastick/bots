#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <utility>

namespace SingularityTrainer
{
class IO;

typedef std::pair<const std::vector<double> &, const std::vector<double> &> IndexedData;

class IndexedDataStore
{
  private:
    std::vector<double> indices;
    std::vector<double> values;

  public:
    void add_data(double timestep, double value);
    void add_data(unsigned long long timestep, double value);
    const IndexedData get_data() const;
};

class TrainInfoWindow
{
  private:
    std::map<std::string, IndexedDataStore> data;
    std::mutex data_mutex;
    IO &io;
    std::string selected_type;

  public:
    TrainInfoWindow(IO &io);

    void add_data(const std::string &label, unsigned long long timestep, double value);
    void update(unsigned long long timestep, unsigned int update);
};
}