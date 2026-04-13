#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "Singleton.h"
#include "Utils/Log.h"
#include "Utils/Path.h"

class AssetLoader
{
  private:
    static inline std::unordered_map<std::filesystem::path, std::shared_ptr<void>> assets;
    static inline const std::filesystem::path ASSET_PATH = [] { return getExecutablePath() / "Assets"; }();

  public:
    template <typename Asset, typename... Args>
    static std::shared_ptr<Asset> get(const std::string_view &path, Args &&...args)
    {
        std::filesystem::path full_path = ASSET_PATH / path;

        auto it = assets.find(full_path);
        if (it == assets.end())
        {
            if (Singleton::gameLoaded)
            {
                LOG_WARNING("resource '{}' should be loaded during program startup", path);
            }

            LOG_DEBUG("loading asset {}", full_path.string());
            assets[full_path] = Asset::load(full_path, std::forward<Args>(args)...);
            LOG_DEBUG("loaded asset {}", full_path.string());
        }

        return std::static_pointer_cast<Asset>(assets[full_path]);
    }
};
