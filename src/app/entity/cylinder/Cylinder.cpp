//
// Created by faliszewskii on 17.06.24.
//

#include "Cylinder.h"


Cylinder::Cylinder() {
    mesh = std::make_unique<Mesh<PosNorTexVertex>>(generateCylinder(0.5f, 1, 15));
}

void Cylinder::render () {
    mesh->render();
}

Mesh<PosNorTexVertex> Cylinder::generateCylinder (float radius, float height, int slices) {
    vertices.clear();
    indices.clear();

    // Generate vertices
    float textureStep = 1.f / static_cast<float>(slices);
    float angleStep = glm::two_pi<float>() / static_cast<float>(slices);
    glm::vec3 topCenter(0.0f, height / 2.0f, 0.0f);
    glm::vec3 bottomCenter(0.0f, -height / 2.0f, 0.0f);

    // Sizes
    int partSize = slices*2;

    // Indices
    int startFaces = slices*2;
    int topMiddle = slices*4;
    int bottomMiddle = slices*4 + 1;

    // Generate side vertices
    for (int i = 0; i <= slices; ++i) {
        float angle = static_cast<float>(i) * angleStep;
        float texCoord = static_cast<float>(i) * textureStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        // Top vertex
        vertices.push_back({glm::vec3(x, height / 2.0f, z), glm::normalize(glm::vec3(x, 0.0f, z)), glm::vec2(texCoord, 0)});

        // Bottom vertex
        vertices.push_back({glm::vec3(x, -height / 2.0f, z), glm::normalize(glm::vec3(x, 0.0f, z)), glm::vec2(texCoord, 1)});
    }

    // Generate side vertices for faces
    for (int i = 0; i <= slices; ++i) {
        float angle = static_cast<float>(i) * angleStep;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        // Top vertex for faces
        vertices.push_back({glm::vec3(x, height / 2.0f, z), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec2()});

        // Bottom vertex for faces
        vertices.push_back({glm::vec3(x, -height / 2.0f, z), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec2()});
    }

    // Add middle points
    // Top vertex for faces
    vertices.push_back({glm::vec3(0, height / 2.0f, 0), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec2()});

    // Bottom vertex for faces
    vertices.push_back({glm::vec3(0, -height / 2.0f, 0), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec2()});


    // Generate indices for the sides
    for (int i = 0; i <= slices * 2-2; i += 2) {
        int nextTop = i + 2 >= partSize ? 0 : i + 2;
        int nextBottom = i + 3 >= partSize ? 1 : i + 3;
        // Triangle 1
        indices.push_back(i);
        indices.push_back(nextBottom);
        indices.push_back(i + 1);

        // Triangle 2
        indices.push_back(i);
        indices.push_back(nextTop);
        indices.push_back(nextBottom);
    }

    // Generate indices for the top and bottom faces
    for (int i = 0; i <= slices; ++i) {
        int nextTop =  (i + 1) * 2 >= partSize ? 0 : (i + 1) * 2;
        int nextBottom = (i + 1) * 2 + 1 >= partSize ? 1 : (i + 1) * 2 + 1;
        // Top face
        indices.push_back(startFaces + i * 2);
        indices.push_back(topMiddle);
        indices.push_back(startFaces + nextTop);

        // Bottom face
        indices.push_back(startFaces + i * 2 + 1);
        indices.push_back(startFaces + nextBottom);
        indices.push_back(bottomMiddle);
    }

    return Mesh<PosNorTexVertex>(vertices, indices);

}
