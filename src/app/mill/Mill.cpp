//
// Created by faliszewskii on 10.10.24.
//

#include "Mill.h"

#include <utility>
#include <expected>
#include "../algorithm/Bresenham.h"

Mill::Mill(float height, float radius, glm::vec3 position, float velocity) {
    type = Spherical;
    this->height = height;
    this->radius = radius;
    this->position = position;
    this->velocity = velocity;
    currentPoint = 0;
    tMillPath = 0;

    millModel = std::make_unique<MillModel>(height, radius, position);
}

std::expected<void, std::string> Mill::advance(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, float deltaTime) {
    if (!path.empty() && currentPoint != path.size() - 1) {
        float amountLeft = velocity * deltaTime;
        while (amountLeft > 0) {
            int currId = currentPoint;
            glm::vec3 currPoint = path[currId];
            glm::vec3 nextPoint = path[currId + 1];
            glm::vec3 direction = nextPoint - currPoint;
            float distance = glm::length(direction);
            float tIncrement = amountLeft / distance;
            float t = tMillPath;
            if (t + tIncrement > 1) {
                auto e = mill(heightMap,baseDimensions, currPoint + t * direction, nextPoint);
                if(!e) return e;
                currentPoint++;
                tMillPath = 0;
                if(currentPoint == path.size()-1) {
                    setPosition(nextPoint);
                    break;
                }
                amountLeft -= distance;
                continue;
            }
            amountLeft = 0;
            tMillPath += tIncrement;
            auto e = mill(heightMap, baseDimensions, currPoint + t * direction, currPoint + tMillPath * direction);
            if(!e) return e;

            setPosition(currPoint + tMillPath * direction);
        }
    }
    return {};
}

std::expected<void, std::string> Mill::mill(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions, glm::vec3 p1, glm::vec3 p2) {
    glm::vec3 direction = p2 - p1;
    std::vector<glm::vec<2, int>> startingPoints;

    int p1Col = (p1.x + baseDimensions.x/2.f) / baseDimensions.x * heightMap[0].size();
    int p1Row = (p1.z + baseDimensions.z/2.f) / baseDimensions.z * heightMap.size();
    int p2Col = (p2.x + baseDimensions.x/2.f) / baseDimensions.x * heightMap[0].size();
    int p2Row = (p2.z + baseDimensions.z/2.f) / baseDimensions.z * heightMap.size();

    int rx = (radius) / baseDimensions.x * heightMap[0].size() + 2;
    int rz = (radius) / baseDimensions.z * heightMap.size() + 2;

    for(int i = p1Col - rx; i <= p1Col + rx; i++) {
        for(int j = p1Row - rz; j <= p1Row + rz; j++) {
            int row = j;
            int col = i;
            if(col >= 0 && col < heightMap[0].size() && row >= 0 && row < heightMap.size()) {
                glm::vec3 gridPosition = glm::vec3(float(col)/heightMap[0].size() * baseDimensions.x - baseDimensions.x/2.f, 0.0, float(row)/heightMap.size() * baseDimensions.z - baseDimensions.z/2.f);
                float pointRadius2 = std::pow(gridPosition.x - p1.x, 2) + std::pow(gridPosition.z - p1.z, 2);
                if(pointRadius2 < radius*radius){
                    float pointHeight = 0;
                    switch(type) {
                        case Flat:
                            pointHeight = p1.y;
                            break;
                        case Spherical:
                            pointHeight = p1.y + radius - sqrt(radius*radius - pointRadius2);
                            break;
                    }
                    if(heightMap[row][col] > pointHeight)
                        heightMap[row][col] = pointHeight;
                }
            }
        }
    }

    glm::vec3 perpendicular = glm::normalize(glm::cross(glm::normalize(direction), glm::vec3(0, 1, 0)));
    glm::vec<2, int> pixelMiddle = glm::vec<2, int>(p1Col, p1Row);
    glm::vec<2, int> pixelBottom = pixelMiddle - glm::vec<2, int>(perpendicular.x* rx, perpendicular.z* rz);
    glm::vec<2, int> pixelTop = pixelMiddle + glm::vec<2, int>(perpendicular.x* rx, perpendicular.z* rz);
    bresenham(pixelBottom.x, pixelBottom.y, pixelTop.x, pixelTop.y, [&](int x, int y, float t) {
        glm::vec3 gridPosition = glm::vec3(float(x)/heightMap[0].size() * baseDimensions.x - baseDimensions.x/2.f, 0.0, float(y)/heightMap.size() * baseDimensions.z - baseDimensions.z/2.f);
        float pointRadius2 = std::pow(gridPosition.x - p1.x, 2) + std::pow(gridPosition.z - p1.z, 2);
        float pointHeight = 0;
        switch(type) {
            case Flat:
                pointHeight = p1.y;
            break;
            case Spherical:
                pointHeight = p1.y + radius - sqrt(radius*radius - pointRadius2);
            break;
        }
        glm::vec<2, int> diff = {p2Col - p1Col, p2Row - p1Row};
        bresenham(x, y, x+diff.x, y+diff.y, [&](int x, int y, float t) {
            if(x < 0 || x >= heightMap[0].size() || y < 0 || y >= heightMap.size())
                return;
            if(heightMap[y][x] > pointHeight)
                heightMap[y][x] = pointHeight;
        });
    });

    return {};
}

void Mill::setHeight(float newHeight) {
    height = newHeight;
    millModel->height = newHeight;
}

void Mill::setRadius(float newRadius) {
    radius = newRadius;
    millModel->radius = newRadius;
}

void Mill::setType(MillType newType) {
    type = newType;
    millModel->type = newType;
}

void Mill::setVelocity(float newVelocity) {
    velocity = newVelocity;
}

void Mill::setPosition(glm::vec3 newPosition) {
    position = newPosition;
    millModel->position = newPosition;
}

void Mill::setPath(std::vector<glm::vec3> newPath) {
    path = std::move(newPath);
    currentPoint = 0;
    tMillPath = 0;
}

void Mill::render(Shader& shader) {
    millModel->render(shader);
}
