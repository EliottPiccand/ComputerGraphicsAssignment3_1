#pragma once

#if defined(TRACY_ENABLE)

#include <GL/glew.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#define ProfilingEndFrame FrameMark
#define ProfileScope ZoneScoped
#define SetGpuProfilingContext TracyGpuContext
#define CollectGpuProfilingEvents TracyGpuCollect
#define ProfileScopeGPU(name) TracyGpuZone(name)

#else

#define ProfilingEndFrame
#define ProfileScope
#define SetGpuProfilingContext
#define CollectGpuProfilingEvents
#define ProfileScopeGPU(name)

#endif
