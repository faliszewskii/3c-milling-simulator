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
    appContext.patchC0Shader = std::make_unique<Shader>(Shader::createTraditionalShader(
                "../res/shaders/patch/patch.vert", "../res/shaders/patch/patch.tesc", "../res/shaders/patch/patch.tese", "../res/shaders/patch/patch.frag"));
    appContext.patchC2Shader = std::make_unique<Shader>(Shader::createTraditionalShader(
                   "../res/shaders/patch/patch.vert", "../res/shaders/patch/patch.tesc", "../res/shaders/patch/patchC2.tese", "../res/shaders/patch/patch.frag"));
    appContext.patchC0ShaderQuad = std::make_unique<Shader>(Shader::createTraditionalShader(
                    "../res/shaders/quadPatch/patch.vert", "../res/shaders/quadPatch/patch.tesc", "../res/shaders/quadPatch/patch.tese", "../res/shaders/quadPatch/patch.frag"));
    appContext.patchC2ShaderQuad = std::make_unique<Shader>(Shader::createTraditionalShader(
                   "../res/shaders/quadPatch/patch.vert", "../res/shaders/quadPatch/patch.tesc", "../res/shaders/quadPatch/patchC2.tese", "../res/shaders/quadPatch/patch.frag"));

    appContext.quad = std::make_unique<Quad>();
    appContext.light = std::make_unique<PointLight>();
    appContext.light->position = {0.0f , 200.0f, 100.f};
    appContext.lightBulb = std::make_unique<Point>();

    appContext.mill = std::make_unique<Mill>(40, 8, glm::vec3(0, 60, 0), 300, 0.999, 10);

    appContext.gCodeParser = std::make_unique<GCodeParser>();

    appContext.pathModel = std::make_unique<Mesh<PositionVertex>>(std::vector<PositionVertex>(), std::nullopt, GL_LINE_STRIP);
    appContext.base = std::make_unique<Mesh<EmptyVertex>>(Mesh<EmptyVertex>({EmptyVertex{}}, std::nullopt, GL_POINTS));

    appContext.baseDimensions = {150, 50, 150};

    appContext.heightMap = std::make_unique<HeightMap>(glm::vec<2, int>(256, 256), appContext.baseDimensions.y);

    appContext.lastFrameTime = 0;
    appContext.running = false;

    appContext.drawPath = true;

    appContext.modelSerializer = std::make_unique<Serializer>();
    appContext.surfaceIntersection = std::make_unique<SurfaceIntersection>();
    appContext.pathGenerator = std::make_unique<PathGenerator>(appContext);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;
}

void Scene::update() {
    // TODO --- Here goes scene data update.
    appContext.lightBulb->position = appContext.light->position;
    appContext.lightBulb->color = glm::vec4(appContext.light->color, 1);

    if(appContext.mill->isThreadRunning() ) {
        appContext.heightMap->update();
    }
    if(appContext.mill->isThreadFinished()) {
        appContext.heightMap->update();
        auto e = appContext.mill->checkError();
        if(e)
            appContext.errorMessages.push_back(e.value());
        appContext.mill->clearError();
    }
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

    glPointSize(4);
    appContext.pointShader->use();
    appContext.pointShader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.pointShader->setUniform("projection", appContext.camera->getProjectionMatrix());
    for(auto &point : appContext.points) {
        point.second.render(*appContext.pointShader);
    }
    for(auto &point : appContext.intersections) {
        point.render(*appContext.pointShader);
    }
    glPointSize(1);

    appContext.patchC0Shader->use();
    appContext.patchC0Shader->setUniform("distance", 0.f);
    appContext.patchC0Shader->setUniform("selected", false);
    appContext.patchC0Shader->setUniform("color", glm::vec4(1));
    appContext.patchC0Shader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.patchC0Shader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.patchC0Shader->setUniform("gridCountLength", 3);
    appContext.patchC0Shader->setUniform("gridCountWidth", 3);

    for(auto &patch : appContext.patchesC0) {
        patch.get().render(*appContext.patchC0Shader);
    }

    appContext.patchC2Shader->use();
    appContext.patchC2Shader->setUniform("distance", 0.f);
    appContext.patchC2Shader->setUniform("selected", false);
    appContext.patchC2Shader->setUniform("color", glm::vec4(1));
    appContext.patchC2Shader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.patchC2Shader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.patchC2Shader->setUniform("gridCountLength", 3);
    appContext.patchC2Shader->setUniform("gridCountWidth", 3);

    for(auto &patch : appContext.patchesC2) {
        patch.get().render(*appContext.patchC2Shader);
    }

    appContext.frameBufferManager->unbind();
}
