#include "Components/Component.h"

#include "GameObject.h"

using namespace component;

void Component::initialize()
{
}

void Component::update(float deltaTime)
{
}

bool Component::render() const
{
    return false;
}

void Component::setOwner(std::shared_ptr<GameObject> gameObject)
{
    owner = gameObject;
}

std::shared_ptr<GameObject> Component::getOwner() const
{
    return owner.lock();
}
