//
// Created by faliszewskii on 16.06.24.
//

#include "Scene.h"
#include "../interface/camera/CameraAnchor.h"
#include "heightmap/HeightMap.h"

Scene::Scene(AppContext &appContext) : appContext(appContext) {
    appContext.camera = std::make_unique<CameraAnchor>(1920, 1080, glm::vec3(0.0f, 300.0f, 300.0f), glm::vec3(0.f), glm::vec3(-M_PI / 4, 0, 0));
    appContext.frameBufferManager = std::make_unique<FrameBufferManager>();
    appContext.frameBufferManager->create_buffers(appContext.camera->screenWidth, appContext.camera->screenHeight);

    // TODO --- Initialization of the app state goes here.

    appContext.phongShader = std::make_unique<Shader>(Shader::createTraditionalShader(
            "../res/shaders/phong/phong.vert", "../res/shaders/phong/phong.frag"));
    appContext.pointShader = std::make_unique<Shader>(Shader::createTraditionalShader(
            "../res/shaders/point/point.vert", "../res/shaders/point/point.frag"));
    appContext.basicShader = std::make_unique<Shader>(Shader::createTraditionalShader(
            "../res/shaders/basic/position.vert", "../res/shaders/basic/white.frag"));
    appContext.millingBaseShader = std::make_unique<Shader>(Shader::createTraditionalShader(
            "../res/shaders/millingBase/millingBase.vert", "../res/shaders/millingBase/millingBase.geom", "../res/shaders/phong/phong.frag"));


    appContext.quad = std::make_unique<Quad>();
    appContext.light = std::make_unique<PointLight>();
    appContext.light->position = {0.0f , 200.0f, 100.f};
    appContext.lightBulb = std::make_unique<Point>();

    appContext.mill = std::make_unique<Mill>(40, 8, glm::vec3(0, 60, 0), 300, 0.999, 10);

    appContext.gCodeParser = std::make_unique<GCodeParser>();

    appContext.pathModel = std::make_unique<Mesh<PositionVertex>>(std::vector<PositionVertex>(), std::nullopt, GL_LINE_STRIP);
    appContext.base = std::make_unique<Mesh<EmptyVertex>>(Mesh<EmptyVertex>({EmptyVertex{}}, std::nullopt, GL_POINTS));

    appContext.baseDimensions = {150, 50, 150};

    appContext.heightMap = std::make_unique<HeightMap>(glm::vec<2, int>(1024, 1024), appContext.baseDimensions.y);

    appContext.lastFrameTime = 0;
    appContext.running = false;

    appContext.drawPath = true;
}

void Scene::update() {
    // TODO --- Here goes scene data update.
    appContext.lightBulb->position = appContext.light->position;
    appContext.lightBulb->color = glm::vec4(appContext.light->color, 1);

    float time = glfwGetTime();
    float deltaTime = time - appContext.lastFrameTime;
    if(appContext.running) {
        auto e = appContext.mill->advance(appContext.heightMap->heightMapData, appContext.baseDimensions, deltaTime);
        if(!e) {
            appContext.running = false;
            appContext.errorMessages.push_back(e.error());
        }

        appContext.heightMap->update();
    }
    appContext.lastFrameTime = time;
}

void Scene::render() {
    appContext.frameBufferManager->bind();

    if(appContext.drawPath) {
        appContext.basicShader->use();
        appContext.basicShader->setUniform("view", appContext.camera->getViewMatrix());
        appContext.basicShader->setUniform("projection", appContext.camera->getProjectionMatrix());
        appContext.basicShader->setUniform("model", glm::identity<glm::mat4>());
        appContext.pathModel->render();
    }

    appContext.millingBaseShader->use();
    appContext.millingBaseShader->setUniform("viewPos", appContext.camera->getViewPosition());
    appContext.millingBaseShader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.millingBaseShader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.millingBaseShader->setUniform("material.hasTexture", false);
    appContext.millingBaseShader->setUniform("material.albedo", glm::vec4(0.5f, 0.6f, 0.7f, 1.0f));
    appContext.millingBaseShader->setUniform("material.shininess", 256.f);
    appContext.millingBaseShader->setUniform("uGridSize", appContext.heightMap->heightMapSize);
    appContext.millingBaseShader->setUniform("uBaseSize", glm::vec2(appContext.baseDimensions.x,appContext.baseDimensions.z));
    appContext.millingBaseShader->setUniform("uHeightScale", appContext.baseDimensions.y);
    appContext.heightMap->heightMap->bind(1);
    appContext.millingBaseShader->setUniform("uHeightMap", 1);
    appContext.light->setupPointLight(*appContext.millingBaseShader);
    appContext.base->render((appContext.heightMap->heightMapSize.x+2) * (appContext.heightMap->heightMapSize.y+2));

    appContext.phongShader->use();
    appContext.phongShader->setUniform("viewPos", appContext.camera->getViewPosition());
    appContext.phongShader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.phongShader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.phongShader->setUniform("model", glm::identity<glm::mat4>());
    appContext.phongShader->setUniform("material.hasTexture", false);
    appContext.phongShader->setUniform("material.albedo", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    appContext.phongShader->setUniform("material.shininess", 256.f);
    appContext.light->setupPointLight(*appContext.phongShader);
    appContext.mill->render(*appContext.phongShader);

    appContext.phongShader->setUniform("material.albedo", glm::vec4(0.5f, 0.6f, 0.7f, 1.0f));
    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::scale(model, appContext.baseDimensions);
    model = glm::rotate(model, float(std::numbers::pi/2.f), glm::vec3(1, 0, 0));
    appContext.phongShader->setUniform("model", model);
    appContext.quad->render();

    appContext.frameBufferManager->unbind();
}
