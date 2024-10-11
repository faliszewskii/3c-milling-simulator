//
// Created by faliszewskii on 17.06.24.
//

#ifndef MATERIAL_PAINTER_CYLINDER_H
#define MATERIAL_PAINTER_CYLINDER_H

#include <memory>
#include "../../../opengl/mesh/PosNorTexVertex.h"
#include "../../../opengl/mesh/Mesh.h"
#include "../../../opengl/shader/Shader.h"

class Cylinder {

    Mesh<PosNorTexVertex> generateCylinder(float radius, float height, int slices);
public:
    std::unique_ptr<Mesh<PosNorTexVertex>> mesh;
    std::vector<PosNorTexVertex> vertices;
    std::vector<unsigned int> indices;

    Cylinder();

    void render();
};

#endif //MATERIAL_PAINTER_CYLINDER_H
