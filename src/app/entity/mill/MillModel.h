//
// Created by faliszewskii on 10.10.24.
//

#ifndef MILLING_SIMULATOR_MILLMODEL_H
#define MILLING_SIMULATOR_MILLMODEL_H


#include <memory>
#include "../cylinder/Cylinder.h"
#include "../sphere/Sphere.h"
#include "../../mill/MillType.h"

class MillModel {

    std::unique_ptr<Sphere> sphere;
    std::unique_ptr<Cylinder> shank;

public:
    MillType type;
    float height;
    float radius;
    glm::vec3 position;


    MillModel(float height, float radius, glm::vec3 position);

    void render(Shader& shader);
};


#endif //MILLING_SIMULATOR_MILLMODEL_H
