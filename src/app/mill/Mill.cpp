//
// Created by faliszewskii on 10.10.24.
//

#include "Mill.h"

#include <utility>
#include <expected>
#include <thread>
#include "../algorithm/Bresenham.h"

Mill::Mill(float height, float radius, glm::vec3 position, float velocity, float minAngleDescend, float minHeight) {
    type = Spherical;
    this->height = height;
    this->radius = radius;
    this->position = position;
    this->velocity = velocity;
    this->maxDescendAngle = minAngleDescend;
    this->minHeight = minHeight;
    currentPoint = 0;
    tMillPath = 0;
    threadRunning = false;
    threadFinished = false;
    stopThreadSignal = false;
    threadError = false;
    errorMessage = "OK";

    millModel = std::make_unique<MillModel>(height, radius, position);
}

void Mill::startInstant(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions) {
    threadRunning = true;
    threadFinished = false;
    stopThreadSignal = false;
    threadError = false;
    std::thread threadObj(&Mill::instantThread, this, std::ref(heightMap), baseDimensions);
    threadObj.detach();
}

void Mill::startMilling(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions) {
    threadRunning = true;
    threadFinished = false;
    stopThreadSignal = false;
    threadError = false;
    std::thread threadObj(&Mill::millThread, this, std::ref(heightMap), baseDimensions);
    threadObj.detach();
}

void Mill::instantThread(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions) {
    for(int i = currentPoint; i < path.size()-1 && !stopThreadSignal; i++) {
        auto e = mill(heightMap,baseDimensions, path[currentPoint], path[currentPoint + 1]);
        if(!e) {
            errorMessage = e.error();
            threadError = true;
            break;
        }
        positionMutex.lock();
        setPosition(path[currentPoint + 1]);
        positionMutex.unlock();
        currentPoint++;
    }
    threadFinished = true;
    threadRunning = false;
}

void Mill::millThread(std::vector<std::vector<float>> &heightMap, glm::vec3 baseDimensions) {
    float deltaTime = 10/1000.f;
    while(!stopThreadSignal && currentPoint != path.size() - 1) {
        auto e = advance(heightMap, baseDimensions, deltaTime);
        if(!e) {
            errorMessage = e.error();
            threadError = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    threadFinished = true;
    threadRunning = false;
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

    if(p2.y < minHeight)
        return std::unexpected("Mill has crossed min height limit");
    float descendAngle = glm::dot(glm::normalize(direction), glm::vec3(0, -1, 0));

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
                    float pointHeight = p1.y;
                    switch(type) {
                        case Flat:
                            break;
                        case Spherical:
                            pointHeight += radius - sqrt(radius*radius - pointRadius2);
                            break;
                    }
                    if(heightMap[row][col] > pointHeight) {
                        if(descendAngle > maxDescendAngle && (type == Flat /*|| (col == p1Col && row == p1Row)*/)) {
                            return std::unexpected("Mill path too steep");
                        }
                        if(heightMap[row][col] - pointHeight > height) {
                            return std::unexpected("Milling with a non-cutting part");
                        }
                        heightMapMutex.lock();
                        heightMap[row][col] = pointHeight;
                        heightMapMutex.unlock();
                    }
                }
            }
        }
    }

    bool error = false;
    std::string errorMessage = "";
    glm::vec3 perpendicular = glm::normalize(glm::cross(glm::normalize(direction), glm::vec3(0, 1, 0)));
    glm::vec<2, int> pixelMiddle = glm::vec<2, int>(p1Col, p1Row);
    glm::vec<2, int> pixelBottom = pixelMiddle - glm::vec<2, int>(perpendicular.x* rx, perpendicular.z* rz);
    glm::vec<2, int> pixelTop = pixelMiddle + glm::vec<2, int>(perpendicular.x* rx, perpendicular.z* rz);
    bresenham(pixelBottom.x, pixelBottom.y, pixelTop.x, pixelTop.y, [&](int x, int y, float str) {
        glm::vec3 gridPosition = glm::vec3(float(x)/heightMap[0].size() * baseDimensions.x - baseDimensions.x/2.f, 0.0, float(y)/heightMap.size() * baseDimensions.z - baseDimensions.z/2.f);
        float pointRadius2 = std::pow(gridPosition.x - p1.x, 2) + std::pow(gridPosition.z - p1.z, 2);
        glm::vec<2, int> diff = {p2Col - p1Col, p2Row - p1Row};
        bresenham(x, y, x+diff.x, y+diff.y, [&](int x, int y, float str) {
            if(x < 0 || x >= heightMap[0].size() || y < 0 || y >= heightMap.size())
                return;
            glm::vec3 gridPositionInner = glm::vec3(float(x)/heightMap[0].size() * baseDimensions.x - baseDimensions.x/2.f, 0.0, float(y)/heightMap.size() * baseDimensions.z - baseDimensions.z/2.f);
            auto AB = glm::vec2(direction.x, direction.z);
            auto AP = glm::vec2(gridPositionInner.x, gridPositionInner.z) - glm::vec2(gridPosition.x, gridPosition.z);
            float lengthSqrAB = AB.x * AB.x + AB.y * AB.y;
            float t = (AP.x * AB.x + AP.y * AB.y) / lengthSqrAB;
            if(t > 1)
                return;
            float pointHeight = (p1 + t * direction).y;
            switch(type) {
                case Flat:
                break;
                case Spherical:
                    pointHeight += radius - sqrt(radius*radius - pointRadius2);
                break;
            }
            if(heightMap[y][x] > pointHeight) {
                if(descendAngle > maxDescendAngle && (type == Flat /*|| (pixelMiddle.x == x && pixelMiddle.y == y)*/)) {
                    std::cout<<descendAngle<<std::endl;
                    std::cout<<p1.y<<std::endl;
                    std::cout<<p2.y<<std::endl;

                    errorMessage = "Mill path too steep";
                    return;
                }
                if(heightMap[y][x] - pointHeight > height) {
                    error = true;
                    errorMessage = "Milling with a non-cutting part";
                    return;
                }
                heightMapMutex.lock();
                heightMap[y][x] = pointHeight;
                heightMapMutex.unlock();
            }
        });
    });
    if(error)
        return std::unexpected(errorMessage);


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

void Mill::setMinDescendAngle(float newMinDescendAngle) {
    maxDescendAngle = newMinDescendAngle;
}

void Mill::setPosition(glm::vec3 newPosition) {
    position = newPosition;
    millModel->mut.lock();
    millModel->position = newPosition;
    millModel->mut.unlock();
}

void Mill::setPath(std::vector<glm::vec3> newPath) {
    path = std::move(newPath);
    currentPoint = 0;
    tMillPath = 0;
}

void Mill::setMinHeight(float newHeight) {
    minHeight = newHeight;
}


float Mill::getHeight() const { return height; }
float Mill::getRadius() const { return radius; }
MillType Mill::getType() const { return type; }
glm::vec3 Mill::getPosition() const { return position; }
float Mill::getVelocity() const { return velocity; }
float Mill::getMinDescendAngle() const { return maxDescendAngle; }
std::vector<glm::vec3> Mill::getPath() const { return path; }
float Mill::getMinHeight() const { return minHeight; }

void Mill::render(Shader& shader) {
    millModel->render(shader);
}

void Mill::reset() {
    tMillPath = 0;
    currentPoint = 0;
}

bool Mill::isThreadRunning() const {
    return threadRunning;
}

bool Mill::isThreadFinished() const {
    return threadFinished;
}

void Mill::signalStop() {
    stopThreadSignal = true;
}

bool Mill::pathFinished() {
    return !path.empty() && currentPoint == path.size() - 1;
}

std::optional<std::string> Mill::checkError() {
    return threadError ? std::optional(errorMessage) : std::nullopt;
}

void Mill::clearError() {
    threadError = false;
    errorMessage = "OK";
}
