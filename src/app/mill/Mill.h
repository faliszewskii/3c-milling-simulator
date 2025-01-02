//
// Created by faliszewskii on 10.10.24.
//

#ifndef MILLING_SIMULATOR_MILL_H
#define MILLING_SIMULATOR_MILL_H


#include <expected>
#include "../entity/mill/MillModel.h"
#include "MillType.h"
#include <atomic>

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

    std::atomic<bool> threadRunning;
    std::atomic<bool> threadFinished;
    std::atomic<bool> stopThreadSignal;
    std::atomic<bool> threadError;
    std::string errorMessage;
    std::mutex heightMapMutex;
    std::mutex positionMutex;

public:
    float pathLength;

    Mill(float height, float radius, glm::vec3 position, float velocity, float minAngleDescend, float minHeight);

    void startInstant(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions);
    void startMilling(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions);

    void instantThread(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions);
    void millThread(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions);

    std::expected<void, std::string> advance(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, float deltaTime);
    std::expected<void, std::string> mill(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, glm::vec3 p1, glm::vec3 p2);

    void signalStop();
    bool pathFinished();
    std::optional<std::string> checkError();
    void clearError();

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
    bool isThreadRunning() const;
    bool isThreadFinished() const;


    void render(Shader& shader);

    void reset();
};


#endif //MILLING_SIMULATOR_MILL_H
