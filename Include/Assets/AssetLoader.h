#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "Singleton.h"
#include "Utils/Log.h"
#include "Utils/Path.h"

class AssetLoader
{
  public:
    template <typename Asset, typename... Args>
    static std::shared_ptr<Asset> get(const std::string_view &path, Args &&...args)
    {
        const std::filesystem::path full_path = ASSET_PATH / path;

        const auto it = assets_.find(full_path);
        if (it == assets_.end())
        {
            if (Singleton::game_loaded)
            {
                LOG_WARNING("resource '{}' should be loaded during program startup", path);
            }

            LOG_DEBUG("loading asset {}", full_path.string());
            assets_[full_path] = Asset::load(full_path, std::forward<Args>(args)...);
            LOG_DEBUG("loaded asset {}", full_path.string());
        }

        return std::static_pointer_cast<Asset>(assets_[full_path]);
    }

  private:
    static inline std::unordered_map<std::filesystem::path, std::shared_ptr<void>> assets_;
    static inline const std::filesystem::path ASSET_PATH = [] { return getExecutablePath() / "Assets"; }();
};
