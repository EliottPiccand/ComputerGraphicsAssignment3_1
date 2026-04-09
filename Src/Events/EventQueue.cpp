#include "Events/EventQueue.h"

void EventQueue::processAll()
{
    for (const auto &eventPtr : events)
    {
        if (!eventPtr)
            continue;

        const event::Event &ref = *eventPtr;
        const std::type_index eventTypeId = typeid(ref);

        const auto it = callbacks.find(eventTypeId);
        if (it == callbacks.end())
            continue;

        for (const auto &wrapper : it->second)
        {
            wrapper(*eventPtr);
        }
    }

    events.clear();
}
