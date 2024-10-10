//
// Created by faliszewskii on 16.06.24.
//

#include "Scene.h"
#include "../interface/camera/CameraAnchor.h"

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
    appContext.light->position = {0.0f , 0.0f, 0.25f};
    appContext.lightBulb = std::make_unique<Point>();

    appContext.gCodeParser = std::make_unique<GCodeParser>();
    appContext.path = std::make_unique<Mesh<PositionVertex>>(std::vector<PositionVertex>(), std::nullopt, GL_LINE_STRIP);
    appContext.base = std::make_unique<Mesh<EmptyVertex>>(Mesh<EmptyVertex>({EmptyVertex{}}, std::nullopt, GL_POINTS));

    appContext.baseDimensions = {150, 50, 150};

    appContext.heightMapSize = glm::vec2(64);
    std::vector<float> t(appContext.heightMapSize.x*appContext.heightMapSize.y);
    for(int i = 0; i < appContext.heightMapSize.x; i++)
        for(int j = 0; j < appContext.heightMapSize.y; j++)
            t[i*appContext.heightMapSize.y + j] = (40 + 10*sin(0.15*i) + 10*cos(0.1*j))/50;
    appContext.heightMap = std::make_unique<Texture>(appContext.heightMapSize.x, appContext.heightMapSize.y, 1, GL_RED, GL_RED, GL_FLOAT, GL_TEXTURE_2D,
                                                     nullptr);
    appContext.heightMap->update2D(t.data());

}

void Scene::update() {
    // TODO --- Here goes scene data update.
    appContext.lightBulb->position = appContext.light->position;
    appContext.lightBulb->color = glm::vec4(appContext.light->color, 1);
}

void Scene::render() {
    appContext.frameBufferManager->bind();

    appContext.basicShader->use();
    appContext.basicShader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.basicShader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.basicShader->setUniform("model", glm::identity<glm::mat4>());
    appContext.pathModel->render();

    appContext.millingBaseShader->use();
    appContext.millingBaseShader->setUniform("viewPos", appContext.camera->getViewPosition());
    appContext.millingBaseShader->setUniform("view", appContext.camera->getViewMatrix());
    appContext.millingBaseShader->setUniform("projection", appContext.camera->getProjectionMatrix());
    appContext.millingBaseShader->setUniform("material.hasTexture", false);
    appContext.millingBaseShader->setUniform("material.albedo", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    appContext.millingBaseShader->setUniform("material.shininess", 256.f);
    appContext.millingBaseShader->setUniform("uGridSize", glm::vec2(1500, 1500));
    appContext.millingBaseShader->setUniform("uBaseSize", glm::vec2(appContext.baseDimensions.x,appContext.baseDimensions.z));
    appContext.millingBaseShader->setUniform("uHeightScale", appContext.baseDimensions.y);
    appContext.heightMap->bind(1);
    appContext.millingBaseShader->setUniform("uHeightMap", 1);
    appContext.light->setupPointLight(*appContext.millingBaseShader);
    appContext.base->render(1500*1500);


    appContext.frameBufferManager->unbind();
}
