//
// Created by USER on 09/11/2024.
//

#ifndef PARAMETRICSURFACE_H
#define PARAMETRICSURFACE_H

#include <concepts>
#include <glm/vec3.hpp>
#include <vector>

// Concept for a parametric surface
template<typename T>
concept ParametricSurface = requires(T surface, float u, float v) {
    { surface.evaluate(u, v) } -> std::convertible_to<glm::vec3>;
    { surface.evaluateDU(u, v) } -> std::convertible_to<glm::vec3>;
    { surface.evaluateDV(u, v) } -> std::convertible_to<glm::vec3>;
    { surface.rangeU() } -> std::convertible_to<float>;
    { surface.rangeV() } -> std::convertible_to<float>;
    { surface.wrapU() } -> std::convertible_to<bool>;
    { surface.wrapV() } -> std::convertible_to<bool>;
};
#endif //PARAMETRICSURFACE_H
