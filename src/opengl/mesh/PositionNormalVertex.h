//
// Created by faliszewskii on 18.04.24.
//

#ifndef PUMAROBOT_POSITIONNORMALVERTEX_H
#define PUMAROBOT_POSITIONNORMALVERTEX_H

#include "../../glew_glfw.h"
#include <vector>
#include "glm/vec3.hpp"

struct PositionNormalVertex{
    glm::vec3 position;
    glm::vec3 normal;

    inline static std::vector<int> getSizes() { return {3, 3}; }
    inline static std::vector<int> getTypes() { return {GL_FLOAT, GL_FLOAT}; }
    inline static std::vector<int> getOffsets() { return {0, offsetof(PositionNormalVertex, normal)}; }
};

#endif //PUMAROBOT_POSITIONNORMALVERTEX_H
