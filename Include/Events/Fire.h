#pragma once

#include <Lib/glm.h>

#include "Events/Event.h"

namespace event
{

struct Fire : public Event
{
    const glm::vec3 position;
    const glm::vec3 initial_velocity;

    Fire(const glm::vec3 &position, const glm::vec3 &initial_velocity);
};

} // namespace event
