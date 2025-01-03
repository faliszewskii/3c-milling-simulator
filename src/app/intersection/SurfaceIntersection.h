//
// Created by USER on 09/11/2024.
//

#ifndef SURFACEINTERSECTION_H
#define SURFACEINTERSECTION_H


#include <expected>

#include "ParametricSurface.h"

class SurfaceIntersection {
public:
    const int subdivisionCountDefault = 64;
    const int subdivisionIterationsDefault = 100;

    const int gradientIterationLimitDefault = 2000;
    const float gradientStartStepDefault = 0.01;
    const int gradientPrecisionEpsilonDefault = 1000;

    const float intersectionPointsDistanceDefault = 0.05;
    const int intersectionIterationsDefault = 1000;
    const int intersectionPrecisionEpsilonDefault = 100;
    const int maxIntersectionPointsDefault = 1000;
    const int smallerStepTriesLimitDefault = 10;
    const float intersectionTangentDotProductDefault = 0.0;

    const float repulserExponentDefault = 0.5;

    int subdivisionCount = subdivisionCountDefault;
    int subdivisionIterations = subdivisionIterationsDefault;

    int gradientIterationLimit = gradientIterationLimitDefault;
    float gradientStartStep = gradientStartStepDefault;
    int gradientPrecisionEpsilon = gradientPrecisionEpsilonDefault;

    float intersectionPointsDistance = intersectionPointsDistanceDefault;
    int intersectionIterations = intersectionIterationsDefault;
    int intersectionPrecisionEpsilon = intersectionPrecisionEpsilonDefault;
    int maxIntersectionPoints = maxIntersectionPointsDefault;
    int smallerStepTriesLimit = smallerStepTriesLimitDefault;
    float intersectionTangentDotProduct = intersectionTangentDotProductDefault;

    float repulserExponent = repulserExponentDefault;

    struct IntersectionResult {
        std::vector<glm::vec3> intersectionPoints;
         std::vector<std::vector<glm::vec2>> surfaces;
        bool wrapped;
    };

    template<ParametricSurface S>
    glm::vec2 getClosestPoint(S &surfaceA, glm::vec3 referencePoint, std::optional<glm::vec2> repulser) {
        float divU = surfaceA.rangeU() / subdivisionCount;
        float divV = surfaceA.rangeV() / subdivisionCount;
        float minU = 0.0f, minV = 0.0f;

        for(int it = 0; it < subdivisionIterations; it++) {
            float minDist = std::numeric_limits<float>::max();
            int bestI = 0, bestJ = 0;

            for(int i = 0; i < subdivisionCount; i++) {
                for(int j = 0; j < subdivisionCount; j++) {
                    float u = minU + divU/2.f + i * divU;
                    float v = minV + divV/2.f + j * divV;
                    float repulsion = repulser.has_value()? std::pow( glm::distance(glm::vec2(u,v), repulser.value()), repulserExponent): 0;
                    float dist = glm::distance(surfaceA.evaluate(u, v), referencePoint) - repulsion;
                    if(dist < minDist) {
                        minDist = dist;
                        bestI = i;
                        bestJ = j;
                    }
                }
            }
            minU += bestI * divU;
            minV += bestJ * divV;
            divU /= subdivisionCount;
            divV /= subdivisionCount;
        }

        return {minU + divU / 2.0f, minV + divV / 2.0f};
    }

    template<ParametricSurface S1, ParametricSurface S2>
    glm::vec4 getGradient(S1 &surfaceA, float u, float v, S2 &surfaceB, float s, float t) {
        glm::vec3 evalA = surfaceA.evaluate(u, v);
        glm::vec3 evalB = surfaceB.evaluate(s, t);
        glm::vec3 evalDiff = evalA - evalB;

        glm::vec3 evalAU = surfaceA.evaluateDU(u, v);
        glm::vec3 evalAV = surfaceA.evaluateDV(u, v);

        glm::vec3 evalBU = surfaceB.evaluateDU(s, t);
        glm::vec3 evalBV = surfaceB.evaluateDV(s, t);

        return {
                glm::dot(evalDiff, evalAU) * 2.0f,
                glm::dot(evalDiff, evalAV) * 2.0f,
                glm::dot(-evalDiff, evalBU) * 2.0f,
                glm::dot(-evalDiff, evalBV) * 2.0f
        };
    }

    bool cap(float &point, float lowerLimit, float upperLimit, bool wrap) {
        if(wrap) {
            while(point < lowerLimit) point += upperLimit - lowerLimit;
            while(point > upperLimit) point -= upperLimit - lowerLimit;
        } else {
            if(point < lowerLimit) {
                point = lowerLimit;
                return true;
            }
            if(point > upperLimit) {
                point = upperLimit;
                return true;
            }
        }
        return false;
    }

    template<ParametricSurface S1, ParametricSurface S2>
    std::expected<IntersectionResult, std::string> findIntersection(S1 &surfaceA, S2 &surfaceB, bool sameSurface, glm::vec3 cursor) {

        auto pointA = getClosestPoint(surfaceA, cursor, {});
        auto pointB = getClosestPoint(surfaceB, cursor, sameSurface ? std::optional(pointA):  std::nullopt);

        float step = gradientStartStep;
        float prevDist = glm::distance(surfaceA.evaluate(pointA.x, pointA.y), surfaceB.evaluate(pointB.x, pointB.y));

        std::vector<glm::vec3> intersectionPoints;
        std::vector<glm::vec2> intersectionUV1;
        std::vector<glm::vec2> intersectionUV2;



        for(int k = 0; k < gradientIterationLimit && prevDist > std::numeric_limits<float>::epsilon()*gradientPrecisionEpsilon; k++) {
            glm::vec4 gradient = getGradient(surfaceA, pointA.x, pointA.y, surfaceB, pointB.x, pointB.y);

            float newU = pointA.x - gradient.x * step;
            float newV = pointA.y - gradient.y * step;
            float newS = pointB.x - gradient.z * step;
            float newT = pointB.y - gradient.w * step;
            cap(newU, 0, surfaceA.rangeU(), surfaceA.wrapU());
            cap(newV, 0, surfaceA.rangeV(), surfaceA.wrapV());
            cap(newS, 0, surfaceB.rangeU(), surfaceB.wrapU());
            cap(newT, 0, surfaceB.rangeV(), surfaceB.wrapV());
            // float currDist = glm::distance(surfaceA.evaluate(pointA.x - gradient.x * step, pointA.y - gradient.y * step), surfaceB.evaluate(pointB.x - gradient.z * step, pointB.y - gradient.w * step));
            float currDist = glm::distance(surfaceA.evaluate(newU, newV), surfaceB.evaluate(newS, newT));
            if(currDist > prevDist) {
                step /= 2;
                prevDist = std::numeric_limits<float>::max();
                continue;
            }
            prevDist = currDist;

            pointA.x = newU;
            pointA.y = newV;
            pointB.x = newS;
            pointB.y = newT;
            step = gradientStartStep;
//            intersectionPoints.push_back(surfaceA.evaluate(newU, newV));
//            intersectionPoints.push_back(surfaceB.evaluate(newS, newT));
        }
//        return IntersectionResult{
//                intersectionPoints,
//                 vec,
//                false
//        };
        if(prevDist > std::numeric_limits<float>::epsilon() * gradientPrecisionEpsilon)
            return std::unexpected<std::string>("Couldn't find an intersection close to the cursor with desired precision");

        intersectionPoints.push_back(surfaceA.evaluate(pointA.x, pointA.y)); // First point on the intersection
        intersectionUV1.emplace_back(pointA.x, pointA.y);
        intersectionUV2.emplace_back(pointB.x, pointB.y);

         float u = pointA.x, v = pointA.y, s = pointB.x, t = pointB.y;
         float startU = u, startV = v, startS = s, startT = t;

         glm::vec3 T = {};
         bool positiveDirection = true;
        bool wasCapped = false;
         for(int k = 0; k < maxIntersectionPoints; k++) {
             if(wasCapped) {
                 if(!positiveDirection) {
                     std::vector<std::vector<glm::vec2>> vec;
                     vec.push_back(intersectionUV1);
                     vec.push_back(intersectionUV2);
                     return IntersectionResult{
                         intersectionPoints,
                          vec,
                         false
                     };
                 }
                 pointA.x = startU;
                 pointA.y = startV;
                 pointB.x = startS;
                 pointB.y = startT;
                 u = startU;
                 v = startV;
                 s = startS;
                 t = startT;
                 positiveDirection = false;
                 T = {};
             }
             wasCapped = false;
             int stepTries = 0;
             float prevDist = std::numeric_limits<float>::max();
             float step = intersectionPointsDistance;
             glm::vec3 p0 = surfaceA.evaluate(u, v);

             glm::vec3 PdU = surfaceA.evaluateDU(u, v);
             glm::vec3 PdV = surfaceA.evaluateDV(u, v);
             glm::vec3 QdU = surfaceB.evaluateDU(s, t);
             glm::vec3 QdV = surfaceB.evaluateDV(s, t);

             glm::vec3 pN = glm::normalize(glm::cross(PdU, PdV));
             glm::vec3 qN = glm::normalize(glm::cross(QdU, QdV));

             auto newT = glm::normalize(glm::cross(pN, qN));
             newT = positiveDirection ? newT : -newT;
             bool signChanged = !(T.x == 0 && T.y == 0 && T.z == 0) && glm::dot(T, newT) < intersectionTangentDotProduct ;
             T = newT;
             T *= signChanged? -1 : 1;
             float currDist;
             for(int it = 0; it < intersectionIterations; it++) {
                 glm::vec3 p = surfaceA.evaluate(u, v);
                 glm::vec3 q = surfaceB.evaluate(s, t);

                 glm::vec3 PdU = surfaceA.evaluateDU(u, v);
                 glm::vec3 PdV = surfaceA.evaluateDV(u, v);
                 glm::vec3 QdU = surfaceB.evaluateDU(s, t);
                 glm::vec3 QdV = surfaceB.evaluateDV(s, t);

                 glm::vec4 f = glm::vec4(p - q, glm::dot(p - p0, T) - step);
                 currDist = glm::length(f);
                 if(currDist <  std::numeric_limits<float>::epsilon() * intersectionPrecisionEpsilon || it == intersectionIterations-1) {
                     if(it == intersectionIterations - 1) {
                         // return std::unexpected<std::string>("Reached intersection iteration limit!");
                         intersectionPoints.push_back(surfaceA.evaluate(pointA.x, pointA.y));
                         intersectionUV1.emplace_back(pointA.x, pointA.y);
                         intersectionPoints.push_back(surfaceA.evaluate(pointB.x, pointB.y));
                         intersectionUV2.emplace_back(pointB.x, pointB.y);
                         std::vector<std::vector<glm::vec2>> vec1;
                         vec1.push_back(intersectionUV1);
                         vec1.push_back(intersectionUV2);
                         return IntersectionResult{
                             intersectionPoints,
                              vec1,
                             true
                         };
                     }
                     pointA.x = u;
                     pointA.y = v;
                     pointB.x = s;
                     pointB.y = t;
                     break;
                 }

                 if(
                     currDist > prevDist) {
                     if(stepTries == smallerStepTriesLimit) {
                         //return std::unexpected<std::string>("Next intersection point Newton diverged");
                          std::vector<std::vector<glm::vec2>> vec;
                          vec.push_back(intersectionUV1);
                          vec.push_back(intersectionUV2);
                         return IntersectionResult{
                             intersectionPoints,
                              vec,
                             false
                         };
                     }
                     stepTries++;
                     step /= 2;
                     prevDist = std::numeric_limits<float>::max();
                     u = pointA.x;
                     v = pointA.y;
                     s = pointB.x;
                     t = pointB.y;

                     wasCapped |= cap(u, 0, surfaceA.rangeU(), surfaceA.wrapU());
                     wasCapped |= cap(v, 0, surfaceA.rangeV(), surfaceA.wrapV());
                     wasCapped |= cap(s, 0, surfaceB.rangeU(), surfaceB.wrapU());
                     wasCapped |= cap(t, 0, surfaceB.rangeV(), surfaceB.wrapV());
                     it = 0;
                     continue;
                 }
                 prevDist = currDist;

                 glm::vec4 FdPu = glm::vec4(PdU, glm::dot(PdU, T));
                 glm::vec4 FdPv = glm::vec4(PdV, glm::dot(PdV, T));
                 glm::vec4 FdQu = glm::vec4(-QdU, 0);
                 glm::vec4 FdQv = glm::vec4(-QdV, 0);

                 glm::mat4 m = glm::mat4(FdPu, FdPv, FdQu, FdQv);
                 m = glm::inverse(m);

                 glm::vec4 x = {u, v, s, t};
                 glm::vec4 dx = m * f;
                 dx.x /= surfaceA.rangeU();
                 dx.y /= surfaceA.rangeV();
                 dx.z /= surfaceB.rangeU();
                 dx.w /= surfaceB.rangeV();
                 x = x - dx;

                 u = x.x;
                 v = x.y;
                 s = x.z;
                 t = x.w;

                 wasCapped |= cap(u, 0, surfaceA.rangeU(), surfaceA.wrapU());
                 wasCapped |= cap(v, 0, surfaceA.rangeV(), surfaceA.wrapV());
                 wasCapped |= cap(s, 0, surfaceB.rangeU(), surfaceB.wrapU());
                 wasCapped |= cap(t, 0, surfaceB.rangeV(), surfaceB.wrapV());
                 if(wasCapped || glm::length(dx) <  std::numeric_limits<float>::epsilon() * intersectionPrecisionEpsilon)
                     break;
             }
             if(positiveDirection) {
                 intersectionPoints.push_back(surfaceA.evaluate(u, v));
                 intersectionUV1.emplace_back(u, v);
                 intersectionUV2.emplace_back(s, t);
             } else {
                 intersectionPoints.insert(intersectionPoints.begin(), surfaceA.evaluate(u, v));
                 intersectionUV1.insert(intersectionUV1.begin(), {u, v});
                 intersectionUV2.insert(intersectionUV2.begin(), {s, t});
             }

             if(intersectionPoints.size() > 2 && glm::length(intersectionPoints.front() - intersectionPoints.back()) < step) {
                 if(!positiveDirection) continue;
                 break;
             }
         }

         if(intersectionPoints.size() == maxIntersectionPoints) return std::unexpected<std::string>("Too many intersection points");
         std::vector<std::vector<glm::vec2>> vec;
         vec.push_back(intersectionUV1);
         vec.push_back(intersectionUV2);
        return IntersectionResult{
            intersectionPoints,
             vec,
            true
        };
    }
};

#endif //SURFACEINTERSECTION_H
