#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Events/Event.h"

class EventQueue
{
  public:
    template <std::derived_from<event::Event> T> using Callback = std::function<void(const T &)>;

  private:
    using GenericCallback = std::function<void(const event::Event &)>;
    
    static inline std::vector<std::unique_ptr<event::Event>> events;
    static inline std::unordered_map<std::type_index, std::vector<GenericCallback>> callbacks;

  public:
    template <std::derived_from<event::Event> EventType, typename... Args> static void post(Args... args)
    {
        events.push_back(std::make_unique<EventType>(args...));
    }

    static void processAll();

    template <std::derived_from<event::Event> T> static void registerCallback(Callback<T> callback)
    {
        const std::type_index typeIdx = typeid(T);

        auto wrapperFunc = [callback](const event::Event &baseEvent) {
            const T *typedEvent = dynamic_cast<const T *>(&baseEvent);
            if (typedEvent)
            {
                callback(*typedEvent);
            }
        };

        callbacks[typeIdx].push_back({std::move(wrapperFunc)});
    }
};
