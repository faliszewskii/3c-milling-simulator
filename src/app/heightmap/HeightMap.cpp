//
// Created by USER on 12/10/2024.
//

#include "HeightMap.h"

#include <GL/glew.h>

HeightMap::HeightMap(glm::vec<2, int> size, float defaultValue){
    resizeHeightMap(size, defaultValue);
}

void HeightMap::resizeHeightMap(glm::vec<2, int> size, float defaultValue) {
    maxValue = defaultValue;
    heightMapData.clear();
    heightMapSize = size;
    for(int i = 0; i < heightMapSize.x; i++) {
        heightMapData.emplace_back();
        for(int j = 0; j < heightMapSize.y; j++)
            heightMapData.back().push_back(defaultValue);
    }
    heightMap = std::make_unique<Texture>(heightMapSize.x, heightMapSize.y, 1, GL_RED, GL_RED,
        GL_FLOAT, GL_TEXTURE_2D, nullptr);

    std::vector<float> t(heightMapSize.x*heightMapSize.y);
    for(int i = 0; i < heightMapSize.x; i++)
        for(int j = 0; j < heightMapSize.y; j++)
            t[j*heightMapSize.x + i] = heightMapData[i][j]/defaultValue;
    heightMap->update2D(t.data());
}

void HeightMap::update() const {
    std::vector<float> t(heightMapSize.x*heightMapSize.y);
    for(int i = 0; i < heightMapSize.x; i++)
        for(int j = 0; j < heightMapSize.y; j++)
            t[j*heightMapSize.x + i] = heightMapData[i][j]/maxValue;
    heightMap->update2D(t.data());
}
