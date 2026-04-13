#include "Components/Water.h"

#include <Lib/OpenGL.h>

#include "Utils/Constants.h"

using namespace component;

bool Water::render() const {    
    constexpr const GLfloat material_ambient[] = {0.0f, 0.1f, 0.3f, 1.0f};
    constexpr const GLfloat matterial_diffuse[] = {0.0f, 0.4f, 0.8f, 0.6f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matterial_diffuse);

    glBegin(GL_QUADS);
        glNormal3f(_v3(UP));

        constexpr const auto V1 = ( NORTH + EAST) / 2.0f;
        constexpr const auto V2 = (-NORTH + EAST) / 2.0f;
        constexpr const auto V3 = (-NORTH - EAST) / 2.0f;
        constexpr const auto V4 = ( NORTH - EAST) / 2.0f;

        glVertex3f(_v3(V1));
        glVertex3f(_v3(V2));
        glVertex3f(_v3(V3));
        glVertex3f(_v3(V4));
    glEnd();

    return false;
}
