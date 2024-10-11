//
// Created by faliszewskii on 07.10.24.
//

#ifndef SPRING_SIMULATION_SPHERE_H
#define SPRING_SIMULATION_SPHERE_H


#include <memory>
#include "../../../opengl/mesh/PosNorTexVertex.h"
#include "../../../opengl/mesh/Mesh.h"
#include "../../../opengl/shader/Shader.h"

class Sphere {

    Mesh<PosNorTexVertex> generateSphere(int parallelCount, int meridianCount);
public:
    std::unique_ptr<Mesh<PosNorTexVertex>> mesh;
    std::vector<PosNorTexVertex> vertices;
    std::vector<unsigned int> indices;

    Sphere();

    void render();
};


#endif //SPRING_SIMULATION_SPHERE_H
