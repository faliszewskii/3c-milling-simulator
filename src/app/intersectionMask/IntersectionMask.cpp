//
// Created by faliszewskii on 28.11.24.
//

#define STB_IMAGE_IMPLEMENTATION
#include <cstring>
#include "IntersectionMask.h"
#include "../io/stb_image.h"

IntersectionMask::IntersectionMask(std::string filename) {
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    mask = std::vector<unsigned char>(width * height * channels);
    std::memcpy(mask.data(), data, mask.size());
}

glm::vec<4, unsigned char> IntersectionMask::sample(float u, float v) {
    int idx = int(u * float(width));
    int idy = int(v * float(height));

    if(idx < 0 || idy < 0 || idx >= width || idy >= height)
        return {0, 0, 0, 0};

    int pos = channels * (idy * width + idx);

    unsigned char r = mask[pos + 0];
    unsigned char g = mask[pos + 1];
    unsigned char b = mask[pos + 2];
    unsigned char a = mask[pos + 3];

    return {r, g, b, a};
}
