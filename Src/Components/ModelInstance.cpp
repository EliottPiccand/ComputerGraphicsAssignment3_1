#include "Components/ModelInstance.h"

#include <cassert>

#include "Utils/Profiling.h"

using namespace component;

ModelInstance::ModelInstance(std::shared_ptr<asset::Model> model, asset::Model::TextureOverride texture_override)
    : model_(model), texture_override_(texture_override)
{
}

bool ModelInstance::render() const
{
    ProfileScope;
    ProfileScopeGPU("ModelInstance::render");

    model_->draw(texture_override_);

    return false;
}
