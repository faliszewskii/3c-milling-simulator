//
// Created by faliszewskii on 28.04.24.
//

#ifndef PUMAROBOT_POSITIONVERTEX_H
#define PUMAROBOT_POSITIONVERTEX_H

// #include "../../glew_glfw.h"
#include <vector>
#include "glm/vec3.hpp"

struct PositionVertex {
    glm::vec3 position;

    inline static std::vector<int> getSizes() { return {3}; }
    inline static std::vector<int> getTypes() { return {GL_FLOAT}; }
    inline static std::vector<int> getOffsets() { return {0}; }
};

#endif //PUMAROBOT_POSITIONVERTEX_H
