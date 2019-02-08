#pragma once

#include <memory>
#include <unordered_map>

namespace SingularityTrainer
{
template <class T>
class AssetStore
{
  public:
    void add(const std::string &id, std::shared_ptr<T> asset);
    std::shared_ptr<T> get(const std::string &id);
    bool check_exists(const std::string &id);

  private:
    std::unordered_map<std::string, std::shared_ptr<T>> asset_map;
};
template <class T>
void AssetStore<T>::add(const std::string &id, std::shared_ptr<T> asset)
{
    // Check for duplicates
    if (check_exists(id))
    {
        return;
    }

    // Add the asset to our map
    asset_map.emplace(id, asset);
};

template <class T>
bool AssetStore<T>::check_exists(const std::string &id)
{
    auto it = asset_map.find(id);
    if (it != asset_map.end())
    {
        return true;
    }

    return false;
};

template <class T>
std::shared_ptr<T> AssetStore<T>::get(const std::string &id)
{
    auto it = asset_map.find(id);
    if (it == asset_map.end())
    {
        return nullptr;
    }

    return it->second;
};
}