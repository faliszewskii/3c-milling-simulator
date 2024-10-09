//
// Created by faliszewskii on 28.04.24.
//

#ifndef PUMAROBOT_EMPTYVERTEX_H
#define PUMAROBOT_EMPTYVERTEX_H

#include "../../glew_glfw.h"
#include <vector>
#include "glm/vec3.hpp"

struct EmptyVertex {
    inline static std::vector<int> getSizes() { return {}; }
    inline static std::vector<int> getTypes() { return {}; }
    inline static std::vector<int> getOffsets() { return {}; }
};

#endif //PUMAROBOT_EMPTYVERTEX_H
