#include "Components/Collider.h"

#include <cstdint>
#include <stdexcept>

#include <Lib/OpenGL.h>

#include "GameObject.h" // IWYU pragma: keep
#include "Singleton.h"
#include "Utils/Color.h"

using namespace component;

Collider::Collider(Type type) : type_(type)
{
}

bool Collider::collideWith(const Collider &other) const
{
    if (!collideWithAABB(other))
    {
        return false;
    }

    if (std::holds_alternative<AABB>(type_) && std::holds_alternative<AABB>(other.type_))
    {
        return true;
    }
    
    throw std::runtime_error("missing collideWith implementation for collider");
}

bool Collider::collideWithAABB(const Collider &other) const
{
    float self_min_x = aabb_.center.x - aabb_.half_size.x;
    float self_max_x = aabb_.center.x + aabb_.half_size.x;
    float self_min_y = aabb_.center.y - aabb_.half_size.y;
    float self_max_y = aabb_.center.y + aabb_.half_size.y;
    float self_min_z = aabb_.center.z - aabb_.half_size.z;
    float self_max_z = aabb_.center.z + aabb_.half_size.z;

    float other_min_x = other.aabb_.center.x - other.aabb_.half_size.x;
    float other_max_x = other.aabb_.center.x + other.aabb_.half_size.x;
    float other_min_y = other.aabb_.center.y - other.aabb_.half_size.y;
    float other_max_y = other.aabb_.center.y + other.aabb_.half_size.y;
    float other_min_z = other.aabb_.center.z - other.aabb_.half_size.z;
    float other_max_z = other.aabb_.center.z + other.aabb_.half_size.z;

    return (self_min_x <= other_max_x && other_min_x <= self_max_x) &&
           (self_min_y <= other_max_y && other_min_y <= self_max_y) &&
           (self_min_z <= other_max_z && other_min_z <= self_max_z);
}

void Collider::initialize()
{
    GET_COMPONENT(Transform, transform_, Collider);
}

void Collider::update(float delta_time)
{
    (void)delta_time;

    const auto transform = transform_.lock()->resolve();

    if (std::holds_alternative<AABB>(type_))
    {
        const AABB &local_aabb = std::get<AABB>(type_);

        glm::vec3 min;
        glm::vec3 max;
        for (uint8_t i = 0; i < 8; ++i)
        {
            const float x = ((i & 1) ? 1.0f : -1.0f) * local_aabb.half_size.x;
            const float y = ((i & 2) ? 1.0f : -1.0f) * local_aabb.half_size.y;
            const float z = ((i & 4) ? 1.0f : -1.0f) * local_aabb.half_size.z;

            const auto corner = glm::vec3(transform * glm::vec4(glm::vec3(x, y, z) + local_aabb.center, 1.0f));

            if (i == 0)
            {
                min = corner;
                max = corner;
            }
            else
            {
                min.x = glm::min(min.x, corner.x);
                min.y = glm::min(min.y, corner.y);
                min.z = glm::min(min.z, corner.z);
                max.x = glm::max(max.x, corner.x);
                max.y = glm::max(max.y, corner.y);
                max.z = glm::max(max.z, corner.z);
            }
        }

        aabb_.half_size = (max - min) * 0.5f;
        aabb_.center = (max + min) * 0.5f;
    }
    else
    {
        throw std::runtime_error("missing update implementation for Collider type");
    }
}

bool Collider::render() const
{
    constexpr const Color AABB_COLOR = rgb(255, 0, 0);
    constexpr const Color OBB_COLOR = rgb(0, 255, 0);
    constexpr const GLfloat LINE_WIDTH = 3.0f;

    if (Singleton::debug)
    {
        GLenum previous_polygon_fill_mode[2];
        glGetIntegerv(GL_POLYGON_MODE, reinterpret_cast<GLint*>(previous_polygon_fill_mode));

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // OBB
        if (std::holds_alternative<AABB>(type_))
        {
            const auto& aabb = std::get<AABB>(type_);

            const auto v1 = glm::vec3( aabb.half_size.x,  aabb.half_size.y, -aabb.half_size.z) + aabb.center;
            const auto v2 = glm::vec3(-aabb.half_size.x,  aabb.half_size.y, -aabb.half_size.z) + aabb.center;
            const auto v3 = glm::vec3(-aabb.half_size.x, -aabb.half_size.y, -aabb.half_size.z) + aabb.center;
            const auto v4 = glm::vec3( aabb.half_size.x, -aabb.half_size.y, -aabb.half_size.z) + aabb.center;
            const auto v5 = glm::vec3( aabb.half_size.x,  aabb.half_size.y,  aabb.half_size.z) + aabb.center;
            const auto v6 = glm::vec3(-aabb.half_size.x,  aabb.half_size.y,  aabb.half_size.z) + aabb.center;
            const auto v7 = glm::vec3(-aabb.half_size.x, -aabb.half_size.y,  aabb.half_size.z) + aabb.center;
            const auto v8 = glm::vec3( aabb.half_size.x, -aabb.half_size.y,  aabb.half_size.z) + aabb.center;

            constexpr const GLfloat material_ambient[] = {_v4(OBB_COLOR)};
            constexpr const GLfloat material_diffuse[] = {_v4(OBB_COLOR)};

            glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

            glLineWidth(LINE_WIDTH);
            glBegin(GL_LINES);
                glVertex3f(_v3(v1)); glVertex3f(_v3(v2));
                glVertex3f(_v3(v2)); glVertex3f(_v3(v3));
                glVertex3f(_v3(v3)); glVertex3f(_v3(v4));
                glVertex3f(_v3(v4)); glVertex3f(_v3(v1));

                glVertex3f(_v3(v5)); glVertex3f(_v3(v6));
                glVertex3f(_v3(v6)); glVertex3f(_v3(v7));
                glVertex3f(_v3(v7)); glVertex3f(_v3(v8));
                glVertex3f(_v3(v8)); glVertex3f(_v3(v5));

                glVertex3f(_v3(v1)); glVertex3f(_v3(v5));
                glVertex3f(_v3(v2)); glVertex3f(_v3(v6));
                glVertex3f(_v3(v3)); glVertex3f(_v3(v7));
                glVertex3f(_v3(v4)); glVertex3f(_v3(v8));
            glEnd();
        }
        else
        {
            throw std::runtime_error("missing render implementation for Collider type");
        }

        // AABB
        {
            glPushMatrix();
            glLoadIdentity();
            Singleton::active_camera.lock()->bind();

            const auto v1 = glm::vec3( aabb_.half_size.x,  aabb_.half_size.y, -aabb_.half_size.z) + aabb_.center;
            const auto v2 = glm::vec3(-aabb_.half_size.x,  aabb_.half_size.y, -aabb_.half_size.z) + aabb_.center;
            const auto v3 = glm::vec3(-aabb_.half_size.x, -aabb_.half_size.y, -aabb_.half_size.z) + aabb_.center;
            const auto v4 = glm::vec3( aabb_.half_size.x, -aabb_.half_size.y, -aabb_.half_size.z) + aabb_.center;
            const auto v5 = glm::vec3( aabb_.half_size.x,  aabb_.half_size.y,  aabb_.half_size.z) + aabb_.center;
            const auto v6 = glm::vec3(-aabb_.half_size.x,  aabb_.half_size.y,  aabb_.half_size.z) + aabb_.center;
            const auto v7 = glm::vec3(-aabb_.half_size.x, -aabb_.half_size.y,  aabb_.half_size.z) + aabb_.center;
            const auto v8 = glm::vec3( aabb_.half_size.x, -aabb_.half_size.y,  aabb_.half_size.z) + aabb_.center;

            constexpr const GLfloat material_ambient[] = {_v4(AABB_COLOR)};
            constexpr const GLfloat material_diffuse[] = {_v4(AABB_COLOR)};

            glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

            glLineWidth(LINE_WIDTH);
            glBegin(GL_LINES);
                glVertex3f(_v3(v1)); glVertex3f(_v3(v2));
                glVertex3f(_v3(v2)); glVertex3f(_v3(v3));
                glVertex3f(_v3(v3)); glVertex3f(_v3(v4));
                glVertex3f(_v3(v4)); glVertex3f(_v3(v1));

                glVertex3f(_v3(v5)); glVertex3f(_v3(v6));
                glVertex3f(_v3(v6)); glVertex3f(_v3(v7));
                glVertex3f(_v3(v7)); glVertex3f(_v3(v8));
                glVertex3f(_v3(v8)); glVertex3f(_v3(v5));

                glVertex3f(_v3(v1)); glVertex3f(_v3(v5));
                glVertex3f(_v3(v2)); glVertex3f(_v3(v6));
                glVertex3f(_v3(v3)); glVertex3f(_v3(v7));
                glVertex3f(_v3(v4)); glVertex3f(_v3(v8));
            glEnd();

            glPopMatrix();
        }

        glPolygonMode(GL_FRONT, previous_polygon_fill_mode[0]);
        glPolygonMode(GL_BACK,  previous_polygon_fill_mode[1]);
    }

    return false;
}
