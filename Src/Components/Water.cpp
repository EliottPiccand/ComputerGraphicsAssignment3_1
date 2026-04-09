#include "Components/Water.h"

#include <GL/glew.h>

#include "Utils/Constants.h"

using namespace component;

bool Water::render() const {    
    constexpr const GLfloat matAmbient[] = {0.0f, 0.1f, 0.3f, 1.0f};
    constexpr const GLfloat matDiffuse[] = {0.0f, 0.4f, 0.8f, 0.6f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);

    glBegin(GL_QUADS);
        glNormal3f(UP.x, UP.y, UP.z);

        constexpr const auto V1 = ( NORTH + EAST) / 2.0f;
        constexpr const auto V2 = (-NORTH + EAST) / 2.0f;
        constexpr const auto V3 = (-NORTH - EAST) / 2.0f;
        constexpr const auto V4 = ( NORTH - EAST) / 2.0f;

        glVertex3f(V1.x, V1.y, V1.z);
        glVertex3f(V2.x, V2.y, V2.z);
        glVertex3f(V3.x, V3.y, V3.z);
        glVertex3f(V4.x, V4.y, V4.z);
    glEnd();

    return false;
}