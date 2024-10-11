//
// Created by faliszewskii on 10.10.24.
//

#include "MillModel.h"

MillModel::MillModel(float height, float radius, glm::vec3 position) {
    type = Spherical;
    this->height = height;
    this->radius = radius;
    this->position = position;
    sphere = std::make_unique<Sphere>();
    shank = std::make_unique<Cylinder>();
}

void MillModel::render(Shader& shader) {
    auto shankModel = glm::identity<glm::mat4>();
    shankModel = glm::translate(shankModel, position + glm::vec3(0, height / 2.f + (type==Spherical?radius / 2.f:0.f), 0));
    shankModel = glm::scale(shankModel, glm::vec3(2*radius, height - (type==Spherical?radius:0.f), 2*radius));

    shader.setUniform("model", shankModel);
    shader.setUniform("material.albedo", glm::vec4(	70.f/255.f, 130.f/255.f, 180.f/255.f, 1.0f));
    shank->render();

    if(type==Spherical) {
        auto sphereModel = glm::identity<glm::mat4>();
        sphereModel = glm::translate(sphereModel, position + glm::vec3(0, radius, 0));
        sphereModel = glm::scale(sphereModel, glm::vec3(radius));

        shader.setUniform("model", sphereModel);
        shader.setUniform("material.albedo", glm::vec4(	70.f/255.f, 130.f/255.f, 180.f/255.f, 1.0f));
        sphere->render();
    }
}
