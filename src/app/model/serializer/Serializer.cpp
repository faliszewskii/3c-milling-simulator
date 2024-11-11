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

void Serializer::importScene(AppContext &appContext, const std::string &path) {
    MG1::SceneSerializer serializer;
    serializer.LoadScene(path);

    auto& scene = MG1::Scene::Get();
    for(auto &point : scene.points) {
        appContext.points[point.GetId()] = Point({point.position.x, point.position.y, point.position.z});
    }

    float maxX = (*std::ranges::max_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.x < p2.position.x; })).position.x;
    float maxY = (*std::ranges::max_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.y < p2.position.y; })).position.y;
    float maxZ = (*std::ranges::max_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.z < p2.position.z; })).position.z;
    float minX = (*std::ranges::min_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.x < p2.position.x; })).position.x;
    float minY = (*std::ranges::min_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.y < p2.position.y; })).position.y;
    float minZ = (*std::ranges::min_element(std::views::values(appContext.points),[](const auto &p1, const auto &p2) { return p1.position.z < p2.position.z; })).position.z;

    float distX = std::max(std::abs(minX), std::abs(maxX));
    float distY = std::max(std::abs(minY), std::abs(maxY));
    float distZ = std::max(std::abs(minZ), std::abs(maxZ));

    float marginXZ = 5;
    float marginY = 0;
    float minBaseHeight = 15;

    float finalX = appContext.baseDimensions.x / 2.f - marginXZ;
    float finalY = appContext.baseDimensions.y - minBaseHeight - marginY;
    float finalZ = appContext.baseDimensions.z / 2.f - marginXZ;

    for(auto &point : std::views::values(appContext.points)) {
        point.position.x = -point.position.x * (finalX / distX);
        point.position.y = point.position.y * (finalY / distY) + minBaseHeight;
        point.position.z = point.position.z * (finalZ / distZ);
    }

    std::map<std::string, std::unique_ptr<PatchC0>*> patchesC0 {
        {"tail", &appContext.tail},
        {"top_fin", &appContext.topFin}
    };

    for(auto &surfaceData : scene.surfacesC0) {
        std::vector<int> points;
        int patchRow = surfaceData.size.x;
        int pointRow = 4;
        for(int j = 0; j < 3 * surfaceData.size.y + 1; j++) {
            for(int i = 0; i < 3 * surfaceData.size.x+1; i++) {
                int patch = (i==0?0 : (i-1)/3) + (j==0?0 : ((j-1)/3) * patchRow);
                int point = (i==0?0 : (i-1)%3+1) + (j==0?0 : ((j-1)%3+1) * pointRow);
                int id =/* idMap[*/surfaceData.patches[patch].controlPoints[point].GetId()/*]*/;
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
                int id = surfaceData.patches[patch].controlPoints[point].GetId();
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

    appContext.surfaceIntersection->gradientPrecisionEpsilon = 5265;
    appContext.surfaceIntersection->intersectionPointsDistance = 0.167;
    glm::vec3 cursor = {3.155, -0.944, -6.565};
    auto result = appContext.surfaceIntersection->findIntersection(*appContext.body, *appContext.wings, false, cursor);
    if(result) {
        auto &points = result->intersectionPoints;
        for(auto &point :points) {
            // appContext.intersections.emplace_back(point);
        }
    }
}
