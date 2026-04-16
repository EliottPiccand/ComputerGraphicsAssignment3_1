#pragma once

#include <concepts>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "Singleton.h"
#include "Utils/Log.h"
#include "Utils/Path.h"

template <typename T>
concept Asset = requires {
    { T::DIRECTORY } -> std::same_as<const std::string_view &>;
    { T::load(std::declval<const std::filesystem::path &>()) } -> std::same_as<std::shared_ptr<T>>;
};

class AssetLoader
{
  public:
    template <Asset A> static std::shared_ptr<A> get(const std::string_view &path)
    {
        const std::filesystem::path full_path = ASSET_PATH / A::DIRECTORY / path;

        const auto it = assets_.find(full_path);
        if (it == assets_.end())
        {
            if (Singleton::game_loaded)
            {
                LOG_WARNING("resource '{}' should be loaded during program startup", path);
            }

            LOG_DEBUG("loading asset {}", full_path.string());
            assets_[full_path] = A::load(full_path);
            LOG_DEBUG("loaded asset {}", full_path.string());
        }

        return std::static_pointer_cast<A>(assets_[full_path]);
    }

  private:
    static inline std::unordered_map<std::filesystem::path, std::shared_ptr<void>> assets_;
    static inline const std::filesystem::path ASSET_PATH = [] { return getExecutablePath() / "Assets"; }();
};
