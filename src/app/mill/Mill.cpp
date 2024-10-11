//
// Created by faliszewskii on 10.10.24.
//

#include "Mill.h"

#include <utility>
#include <expected>

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

    // TODO Spherical
    // TODO Check pixel rectangle vs sphere collision
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

//    for(int row = 0; row < heightMap.size(); row++) {
//        for(int col = 0; col < heightMap[0].size(); col++) {
//
//
//            glm::vec3 gridPosition = glm::vec3(float(col)/heightMap[0].size() * baseDimensions.x - baseDimensions.x/2.f, 0.0, float(row)/heightMap.size() * baseDimensions.z - baseDimensions.z/2.f);
//            // TODO For now check only if middle(lefttop) inside a circle
//
//            if(std::pow(gridPosition.x - p1.x, 2) + std::pow(gridPosition.z - p1.z, 2) > radius*radius)
//                continue;
//
//            if(heightMap[row][col] > p1.y)
//                heightMap[row][col] = p1.y;
//            // TODO Add spherical mill
////            auto AB = direction;
////            auto AP = P - p1;
////            float lengthSqrAB = AB.x * AB.x + AB.y * AB.y;
////            float t = (AP.x * AB.x + AP.y * AB.y) / lengthSqrAB;
////            float height = (p1 + t * direction).y;
//        }
//    }
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
