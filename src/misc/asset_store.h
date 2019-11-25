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
    void add_raw(const std::string &id, const T *asset);
    const T *get(const std::string &id);
    bool check_exists(const std::string &id);

  private:
    std::unordered_map<std::string, std::shared_ptr<T>> asset_map;
    std::unordered_map<std::string, const T *> raw_asset_map;
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
}

template <class T>
void AssetStore<T>::add_raw(const std::string &id, const T *asset)
{
    // Check for duplicates
    if (check_exists(id))
    {
        return;
    }

    // Add the asset to our map
    raw_asset_map.emplace(id, asset);
}

template <class T>
bool AssetStore<T>::check_exists(const std::string &id)
{
    auto it = asset_map.find(id);
    if (it != asset_map.end())
    {
        return true;
    }

    auto raw_it = raw_asset_map.find(id);
    if (raw_it != raw_asset_map.end())
    {
        return true;
    }

    return false;
}

template <class T>
const T *AssetStore<T>::get(const std::string &id)
{
    auto it = asset_map.find(id);
    if (it != asset_map.end())
    {
        return it->second.get();
    }

    auto raw_it = raw_asset_map.find(id);
    if (raw_it != raw_asset_map.end())
    {
        return raw_it->second;
    }

    return nullptr;
}
}