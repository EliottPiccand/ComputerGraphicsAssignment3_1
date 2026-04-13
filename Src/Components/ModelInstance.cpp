#include "Components/ModelInstance.h"

#include <cassert>

#include "Utils/Profiling.h"

using namespace component;

ModelInstance::ModelInstance(std::shared_ptr<asset::Model> model) : model(model)
{
}

bool ModelInstance::render() const
{
    ProfileScope;
    ProfileScopeGPU("ModelInstance::render");

    model->draw();

    return false;
}
