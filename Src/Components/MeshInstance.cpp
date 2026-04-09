#include "Components/MeshInstance.h"

#include <cassert>
#include <memory>

#include "Assets/Mesh.h"
#include "Utils/Profiling.h"

using namespace component;

MeshInstance::MeshInstance(std::shared_ptr<Mesh> mesh) : mesh(mesh)
{
}

bool MeshInstance::render() const
{
    ProfileScope;
    ProfileScopeGPU("MeshInstance::render");

    mesh->draw();

    return false;
}
