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
concept FileAsset = requires {
    { T::DIRECTORY } -> std::same_as<const std::string_view &>;
    { T::loadFromFile(std::declval<const std::filesystem::path &>()) } -> std::same_as<std::shared_ptr<T>>;
};

class AssetLoader
{
  public:
    template <FileAsset A> static std::shared_ptr<A> getOrLoadFromFile(const std::filesystem::path &path)
    {
        const std::filesystem::path full_path = ASSET_PATH / A::DIRECTORY / path;
        const std::string name = path.string();

        const auto it = assets_.find(name);
        if (it == assets_.end())
        {
            if (Singleton::game_loaded)
            {
                LOG_WARNING("asset '{}' should be loaded during program startup", name);
            }

            LOG_DEBUG("loading asset {}", name);
            assets_[name] = A::loadFromFile(full_path);
            LOG_DEBUG("loaded asset {}", name);
        }

        return std::static_pointer_cast<A>(assets_[name]);
    }

  private:
    static inline std::unordered_map<std::string, std::shared_ptr<void>> assets_;
    static inline const std::filesystem::path ASSET_PATH = [] { return getExecutablePath() / "Assets"; }();
};
