#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "Components/Component.h"

using GameObjectId = size_t;

class GameObject : public std::enable_shared_from_this<GameObject>
{
  private:
    inline static GameObjectId nextId = 0;
    const GameObjectId id;

    std::vector<std::shared_ptr<component::Component>> components;

    std::optional<std::weak_ptr<GameObject>> parent;
    std::vector<std::shared_ptr<GameObject>> children;

    bool initialized = false;

  public:
    bool visible = true;
    bool active = true;

    GameObject();

    void initialize();
    void update(float deltaTime);
    void render() const;

    GameObjectId getId() const;

    std::shared_ptr<GameObject> addChild();
    std::optional<std::shared_ptr<GameObject>> getParent() const;

    std::optional<std::shared_ptr<GameObject>> getGameObject(GameObjectId id);
    void detach();

    template <std::derived_from<component::Component> T, typename... Args>
    std::shared_ptr<T> addComponent(Args &&...args)
    {
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        component->setOwner(shared_from_this());
        components.push_back(component);
        return component;
    }

    template <std::derived_from<component::Component> T>
    [[nodiscard]] std::optional<std::shared_ptr<T>> getComponent() const
    {
        for (auto &component : components)
        {
            if (auto result = std::dynamic_pointer_cast<T>(component))
            {
                return std::optional(result);
            }
        }
        return std::nullopt;
    }

    template <std::derived_from<component::Component> T> bool removeComponent()
    {
        for (auto it = components.begin(); it != components.end(); ++it)
        {
            if (std::dynamic_pointer_cast<T>(it))
            {
                components.erase(it);
                return true;
            }
        }
        return false;
    }

    template <std::derived_from<component::Component> T>
    std::optional<std::shared_ptr<T>> findFirstComponentInParents(bool searchSelf = true) const
    {
        auto nodeOption = searchSelf ? std::optional(shared_from_this()) : (parent.has_value() ? std::optional(parent->lock()) : std::nullopt);
        while (nodeOption.has_value())
        {
            const auto &node = nodeOption.value();

            const auto componentOption = node->getComponent<T>();
            if (componentOption.has_value())
            {
                return componentOption;
            }
            else
            {
                nodeOption = (node->parent.has_value() ? std::optional(node->parent->lock()) : std::nullopt);
            }
        }

        return std::nullopt;
    }
};
