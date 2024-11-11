//
// Created by USER on 10/11/2024.
//

#ifndef TOPDOWNCAMERA_H
#define TOPDOWNCAMERA_H
#include "BaseCamera.h"

class TopDownCamera: BaseCamera {
public:
    [[nodiscard]] glm::mat4 getViewMatrix() const override {
        glm::vec3 up = glm::vec3(0, 0, 1);
        glm::vec3 eye = glm::vec3(0, -1, 0);
        glm::vec3 center = glm::vec3(0, 0, 0);
        return glm::lookAt(eye, center, up);
    }

    [[nodiscard]] glm::mat4 getNoTranslationViewMatrix() const override {
        return glm::identity<glm::mat4>();
    };

    [[nodiscard]] glm::mat4 getMirrorViewMatrix() const override {
        return glm::identity<glm::mat4>();
    };

    [[nodiscard]] glm::mat4 getProjectionMatrix() const override {
        return glm::ortho(-75.0f, 75.0f, -75.0f, 75.0f, 50.0f, 15.0f);
    };

    [[nodiscard]] glm::vec3 getViewPosition() const override {
        return glm::vec3(0, 100, 0);
    };

    [[nodiscard]] float getFarPlane() const override {
        return 15.0f;
    };

    [[nodiscard]] float getNearPlane() const override {
        return 50.0f;
    };

    void processKeyboard(CameraMovement direction, float deltaTime) override {};

    void processMouseMovement(float xoffset, float yoffset) override {};

    void processMouseScroll(float yoffset) override {};

    void resize(int screenWidth, int screenHeight) override {};
};

#endif //TOPDOWNCAMERA_H
