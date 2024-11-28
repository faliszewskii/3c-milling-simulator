//
// Created by faliszewskii on 28.11.24.
//

#ifndef MILLING_SIMULATOR_INTERSECTIONMASK_H
#define MILLING_SIMULATOR_INTERSECTIONMASK_H


#include <vector>
#include <string>
#include <glm/detail/type_vec4.hpp>
#include <glm/vec4.hpp>

class IntersectionMask {

    std::vector<unsigned char> mask;

public:

    explicit IntersectionMask(std::string filename);

    glm::vec<4, unsigned char> sample(float u, float v);

    int width;
    int height;
    int channels;
};


#endif //MILLING_SIMULATOR_INTERSECTIONMASK_H
