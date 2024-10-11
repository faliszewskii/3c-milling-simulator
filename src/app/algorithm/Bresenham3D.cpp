//
// Created by faliszewskii on 10.10.24.
//

#include "Bresenham3D.h"

#include <cmath>
#include <vector>
using namespace std;

vector<vector<int> > Bresenham3D::bresenham3D(int x1, int y1, int z1, int x2, int y2, int z2)
{
    vector<vector<int> > ListOfPoints;
    ListOfPoints.push_back({ x1, y1, z1 });
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int dz = abs(z2 - z1);
    int xs;
    int ys;
    int zs;
    if (x2 > x1)
        xs = 1;
    else
        xs = -1;
    if (y2 > y1)
        ys = 1;
    else
        ys = -1;
    if (z2 > z1)
        zs = 1;
    else
        zs = -1;

// Driving axis is X-axis"
    if (dx >= dy && dx >= dz) {
        int p1 = 2 * dy - dx;
        int p2 = 2 * dz - dx;
        while (x1 != x2) {
            x1 += xs;
            if (p1 >= 0) {
                y1 += ys;
                p1 -= 2 * dx;
            }
            if (p2 >= 0) {
                z1 += zs;
                p2 -= 2 * dx;
            }
            p1 += 2 * dy;
            p2 += 2 * dz;
            ListOfPoints.push_back({ x1, y1, z1 });
        }

        // Driving axis is Y-axis"
    }
    else if (dy >= dx && dy >= dz) {
        int p1 = 2 * dx - dy;
        int p2 = 2 * dz - dy;
        while (y1 != y2) {
            y1 += ys;
            if (p1 >= 0) {
                x1 += xs;
                p1 -= 2 * dy;
            }
            if (p2 >= 0) {
                z1 += zs;
                p2 -= 2 * dy;
            }
            p1 += 2 * dx;
            p2 += 2 * dz;
            ListOfPoints.push_back({ x1, y1, z1 });
        }

        // Driving axis is Z-axis"
    }
    else {
        int p1 = 2 * dy - dz;
        int p2 = 2 * dx - dz;
        while (z1 != z2) {
            z1 += zs;
            if (p1 >= 0) {
                y1 += ys;
                p1 -= 2 * dz;
            }
            if (p2 >= 0) {
                x1 += xs;
                p2 -= 2 * dz;
            }
            p1 += 2 * dy;
            p2 += 2 * dx;
            ListOfPoints.push_back({ x1, y1, z1 });
        }
    }
    return ListOfPoints;
}

// This code is contributed by ishankhandelwals.
