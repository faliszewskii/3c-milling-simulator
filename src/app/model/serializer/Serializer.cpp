//
// Created by faliszewskii on 12.05.24.
//

#include "Serializer.h"

#include <ranges>
#include <algorithm>

#include "../../AppContext.h"
#include "../../entity/patchC0/PatchC0.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"

std::vector<unsigned int> getPatchIndices(const bool wrapped, const bool C2, const int patchCountWidth, const int patchCountLength) {
    int vert_n = C2 ? patchCountWidth + 3 : 3 * patchCountWidth + 1;
    int wrappedOverlap = C2 ? 3 : 1;
    vert_n -= wrapped ? wrappedOverlap : 0;

    int step = C2 ? 1 : 3;
    std::vector<unsigned int> indices;
    for(int m = 0; m < patchCountLength; m++) {
        for(int n = 0; n < patchCountWidth; n++) {
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    int k = i + step * n;
                    int l = j + step * m;
                    //                    int t = n - params.patchCountWidth + wrappedOverlap - 3 + i;
                    if(wrapped && k >= vert_n) {
                        indices.push_back(l * vert_n + (k - vert_n));
                    } else {
                        indices.push_back(k + l * vert_n);
                    }
                }
            }
        }
    }
    return indices;
}

void Serializer::loadPatches(AppContext &appContext, MG1::Scene &scene, std::map<int, int> &idMap) const {
    std::map<std::string, std::unique_ptr<PatchC0>*> patchesC0 {
        {"tail", &appContext.tail},
        {"top_fin", &appContext.topFin},
        {"planeXZ", &appContext.planeXZ}
    };

    for(auto &surfaceData : scene.surfacesC0) {
        std::vector<int> points;
        int patchRow = surfaceData.size.x;
        int pointRow = 4;
        for(int j = 0; j < 3 * surfaceData.size.y + 1; j++) {
            for(int i = 0; i < 3 * surfaceData.size.x+1; i++) {
                int patch = (i==0?0 : (i-1)/3) + (j==0?0 : ((j-1)/3) * patchRow);
                int point = (i==0?0 : (i-1)%3+1) + (j==0?0 : ((j-1)%3+1) * pointRow);
                int id = idMap[surfaceData.patches[patch].controlPoints[point].GetId()];
                points.push_back(id);
            }
        }

        std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;
        std::vector<PositionVertex> vertices;
        for(auto &pointId :points) {
            auto &point = appContext.points[pointId];
            vertices.push_back({point.position});
            controlPoints.emplace_back(pointId, point);
        }

        bool wrapped = surfaceData.uWrapped || surfaceData.vWrapped;

        auto patchCountWidth = static_cast<int>(surfaceData.size.x);
        auto patchCountLength = static_cast<int>(surfaceData.size.y);
        auto indices = getPatchIndices(false, false, patchCountWidth, patchCountLength);
        auto patch = PatchC0(vertices, indices, patchCountWidth, patchCountLength, wrapped);
        patch.controlPoints = controlPoints;

        *patchesC0[surfaceData.name] = std::make_unique<PatchC0>(patch);
        appContext.patchesC0.emplace_back(**patchesC0[surfaceData.name]);
    }

    std::map<std::string, std::unique_ptr<PatchC2>*> patchesC2 {
            {"bottom_eye", &appContext.bottomEye},
            {"bottom_inner", &appContext.bottomInner},
            {"top_inner", &appContext.topInner},
            {"top_eye", &appContext.topEye},
            {"body", &appContext.body},
            {"butt", &appContext.butt},
            {"nose", &appContext.nose},
            {"wings", &appContext.wings},
            {"bottom_fin", &appContext.bottomFin},

    };

    for(auto &surfaceData : scene.surfacesC2) {
        std::vector<int> points;
        int patchRow = surfaceData.size.x;
        int pointRow = 4;
        for(int j = 0; j < 3 + surfaceData.size.y; j++) {
            for(int i = 0; i < surfaceData.size.x +3/*+ (surfaceData.uWrapped || surfaceData.vWrapped ? 0 : 3)*/; i++) {
                int patch = (i<3?0 : (i-3)) + (j<3?0 : (j-3) * patchRow);
                int point = (i<3?i : 3) + (j<3?j : 3) * pointRow;
                int id = idMap[surfaceData.patches[patch].controlPoints[point].GetId()];
                points.push_back(id);
            }
        }

        std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;
        std::vector<PositionVertex> vertices;
        for(auto &pointId :points) {
            auto &point = appContext.points[pointId];
            vertices.push_back({point.position});
            controlPoints.emplace_back(pointId, point);
        }

        bool wrapped = surfaceData.uWrapped || surfaceData.vWrapped;

        auto patchCountWidth = static_cast<int>(surfaceData.size.x);
        auto patchCountLength = static_cast<int>(surfaceData.size.y);
        auto indices = getPatchIndices(false, true, patchCountWidth, patchCountLength);
        auto patch = PatchC2(vertices, indices, patchCountWidth, patchCountLength, wrapped);
        patch.controlPoints = controlPoints;
        *patchesC2[surfaceData.name] = std::make_unique<PatchC2>(patch);
        appContext.patchesC2.emplace_back(**patchesC2[surfaceData.name]);
    }
}

void Serializer::transformPoints(const AppContext &appContext, std::map<int, glm::vec3> &map) const {
    for(auto &point : std::views::values(map)) {
        point = appContext.modelTransform * glm::vec4(point.x, point.y, point.z, 1.0f);
    }
}

std::map<int, int> Serializer::uploadPoints(AppContext &appContext, std::map<int, glm::vec3> points) const {
    std::map<int, int> pointMapping;
    for(auto &[id, point] : points) {
        int nextId = appContext.points.size();
        pointMapping[id] = nextId;
        appContext.points[nextId] = Point({point.x, point.y, point.z});
    }
    return pointMapping;
}

MG1::Scene& Serializer::getScene(const std::string &path) const {
    MG1::SceneSerializer serializer;
    serializer.LoadScene(path);
    return MG1::Scene::Get();
}

std::map<int, glm::vec3> Serializer::loadPoints(MG1::Scene &scene) const {
    std::map<int, glm::vec3> points;
    for(auto &point : scene.points) {
        points[point.GetId()] = {point.position.x, point.position.y, point.position.z};
    }
    return points;
}

void Serializer::generateTransform(AppContext &appContext, std::map<int, glm::vec3>& points) const {
    float maxX = (*std::ranges::max_element(std::views::values(points), [](const auto &p1, const auto &p2) { return p1.x < p2.x; })).x;
    float maxY = (*std::ranges::max_element(std::views::values(points),[](const auto &p1, const auto &p2) { return p1.y < p2.y; })).y;
    float maxZ = (*std::ranges::max_element(std::views::values(points),[](const auto &p1, const auto &p2) { return p1.z < p2.z; })).z;
    float minX = (*std::ranges::min_element(std::views::values(points),[](const auto &p1, const auto &p2) { return p1.x < p2.x; })).x;
    float minY = (*std::ranges::min_element(std::views::values(points),[](const auto &p1, const auto &p2) { return p1.y < p2.y; })).y;
    float minZ = (*std::ranges::min_element(std::views::values(points),[](const auto &p1, const auto &p2) { return p1.z < p2.z; })).z;

    float distX = std::max(std::abs(minX), std::abs(maxX));
    float distY = std::max(std::abs(minY), std::abs(maxY));
    float distZ = std::max(std::abs(minZ), std::abs(maxZ));

    float marginXZ = 5.0f;
    float marginY = 0.0f;
    float minBaseHeight = 15.0f;

    float finalX = appContext.baseDimensions.x / 2.0f - marginXZ;
    float finalY = appContext.baseDimensions.y - minBaseHeight - marginY;
    float finalZ = appContext.baseDimensions.z / 2.0f - marginXZ;

    float scaleX = -finalX / distX;
    float scaleY = finalY / distY;
    float scaleZ = finalZ / distZ;
    float translateY = minBaseHeight;

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, translateY, 0.0f));
    transform = glm::scale(transform, glm::vec3(scaleX, scaleY, scaleZ));
    appContext.modelTransform = transform;
}

void Serializer::importModel(AppContext &appContext, const std::string &path) {
    MG1::Scene &scene = getScene(path);
    auto scenePoints = loadPoints(scene);
    generateTransform(appContext, scenePoints);
    transformPoints(appContext, scenePoints);
    auto idMap = uploadPoints(appContext, scenePoints);
    loadPatches(appContext, scene, idMap);
}

void Serializer::importHelper(AppContext &appContext, const std::string &fpath, std::vector<std::string> outlineNames) {
    MG1::Scene &scene = getScene(fpath);
    auto scenePoints = loadPoints(scene);
    transformPoints(appContext, scenePoints);
    auto idMap = uploadPoints(appContext, scenePoints);


    for(int i = 0; i < outlineNames.size(); i++) {
        std::vector<glm::vec3> path;
        auto outlineData = scene.interpolatedC2[i];
        for(auto &ctrlPtr : outlineData.controlPoints) {
            auto &p = appContext.points[idMap[ctrlPtr.GetId()]];
            path.emplace_back(p.position.x, p.position.y, p.position.z);
        }
        appContext.outlines[outlineNames[i]] = path;
    }
}
