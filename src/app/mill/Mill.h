//
// Created by faliszewskii on 10.10.24.
//

#ifndef MILLING_SIMULATOR_MILL_H
#define MILLING_SIMULATOR_MILL_H


#include <expected>
#include "../entity/mill/MillModel.h"
#include "MillType.h"

class Mill {
    std::unique_ptr<MillModel> millModel;

    MillType type;
    float height;
    float radius;
    glm::vec3 position;
    float velocity;
    float maxDescendAngle;
    float minHeight;

    std::vector<glm::vec3> path;
    int currentPoint;
    float tMillPath;

public:
    Mill(float height, float radius, glm::vec3 position, float velocity, float minAngleDescend, float minHeight);

    std::expected<void, std::string> instant(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions);
    std::expected<void, std::string> advance(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, float deltaTime);
    std::expected<void, std::string> mill(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, glm::vec3 p1, glm::vec3 p2);

    void setHeight(float newHeight);
    void setRadius(float newRadius);
    void setType(MillType newType);
    void setPosition(glm::vec3 newPosition);
    void setVelocity(float newVelocity);
    void setMinDescendAngle(float newMinDescendAngle);
    void setPath(std::vector<glm::vec3> newPath);
    void setMinHeight(float newHeight);

    float getHeight() const;
    float getRadius() const;
    MillType getType() const;
    glm::vec3 getPosition() const;
    float getVelocity() const;
    float getMinDescendAngle() const;
    std::vector<glm::vec3> getPath() const;
    float getMinHeight() const;


    void render(Shader& shader);

    void reset();
};


#endif //MILLING_SIMULATOR_MILL_H
