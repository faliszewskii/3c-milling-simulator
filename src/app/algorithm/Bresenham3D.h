//
// Created by faliszewskii on 10.10.24.
//

#ifndef MILLING_SIMULATOR_BRESENHAM3D_H
#define MILLING_SIMULATOR_BRESENHAM3D_H


#include <vector>

class Bresenham3D {
    static std::vector<std::vector<int>> bresenham3D(int x1, int y1, int z1, int x2, int y2, int z2);
};


#endif //MILLING_SIMULATOR_BRESENHAM3D_H
