#pragma once

#include <concepts>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "Utils/Log.h"
#include "Utils/Path.h"

template <typename T>
concept Asset = requires(const std::filesystem::path &path) {
    { T::load(path) } -> std::same_as<std::shared_ptr<T>>;
};

class AssetLoader
{
  private:
    static inline std::unordered_map<std::filesystem::path, std::shared_ptr<void>> assets;
    static inline const std::filesystem::path ASSET_PATH = [] { return getExecutablePath() / "Assets"; }();

  public:
    template <Asset T> static void load(const std::string &path)
    {
        std::filesystem::path full_path = ASSET_PATH / path;

        auto it = assets.find(full_path);
        if (it != assets.end())
        {
            LOG_WARNING("resource {} already loaded", path);
            return;
        }

        auto asset = T::load(full_path);
        assets[full_path] = asset;
    }

    template <Asset T> static std::shared_ptr<T> get(const std::string &path)
    {
        std::filesystem::path full_path = ASSET_PATH / path;

        auto it = assets.find(full_path);
        if (it == assets.end())
        {
            LOG_WARNING("resource {} should be loaded during program startup", path);
            auto asset = T::load(full_path);
            assets[full_path] = asset;
        }

        return std::static_pointer_cast<T>(assets[full_path]);
    }
};
