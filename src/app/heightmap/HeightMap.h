//
// Created by USER on 12/10/2024.
//

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../../opengl/texture/Texture.h"


struct HeightMap {
    std::vector<std::vector<float>> heightMapData;
    std::unique_ptr<Texture> heightMap;
    glm::vec<2, int> heightMapSize;
    float maxValue = 0;

    HeightMap(glm::vec<2, int> size, float defaultValue);

    void resizeHeightMap(glm::vec<2, int> size, float defaultValue);

    void update() const;
};



#endif //HEIGHTMAP_H
