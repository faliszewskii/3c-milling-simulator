//
// Created by faliszewskii on 16.06.24.
//

#include <ranges>
#include <algorithm>
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
            "../res/shaders/millingBase/millingBase.vert", "../res/shaders/millingBase/millingBase.geom", "../res/shaders/millingBase/millingBase.frag"));
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
    appContext.gCodeExporter = std::make_unique<GCodeExporter>();

    appContext.pathModel = std::make_unique<Mesh<PositionVertex>>(std::vector<PositionVertex>(), std::nullopt, GL_LINE_STRIP);
    appContext.base = std::make_unique<Mesh<EmptyVertex>>(Mesh<EmptyVertex>({EmptyVertex{}}, std::nullopt, GL_POINTS));

    appContext.baseDimensions = {150, 50, 150};

    appContext.heightMap = std::make_unique<HeightMap>(glm::vec<2, int>(256, 256), appContext.baseDimensions.y);

    appContext.lastFrameTime = 0;
    appContext.running = false;

    appContext.drawPath = true;
    appContext.drawMeshes = true;
    appContext.drawMill = true;
    appContext.useColorMap = false;
    appContext.errorMargin = 1;

    appContext.modelSerializer = std::make_unique<Serializer>();
    appContext.surfaceIntersection = std::make_unique<SurfaceIntersection>();
    appContext.pathGenerator = std::make_unique<PathGenerator>(appContext);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;

    appContext.everyNthPathPoint = 1;

    // TODO DEBUG
    appContext.modelSerializer->importModel(appContext, "../res/models/fish_final13.json");

    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/body_outline.json",
                                             {"body_bottom", "body_top"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/bottom_fin_outline.json",
                                             {"bottom_fin_top", "bottom_fin_bottom"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/eye_outline.json",
                                             {"eye_top", "eye_bottom"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/nose_outline.json",
                                             {"nose_top", "nose_bottom"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/tail_outline.json",
                                             {"tail1", "tail2", "tail3", "tail4", "tail5", "tail6", "tail7", "tail8"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/wings_outline.json",
                                             {"wings"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/inside_outline.json",
                                             {"inside_top", "inside_bottom"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/butt_outline.json",
                                             {"butt1", "butt3", "butt2"});
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/fin_outline.json",
                                             {"fin"});

    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/bodyWings.json",
{"bodyWings"}, false);
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/body_fin.json",
{"body_fin1", "body_fin3", "body_fin4", "body_fin5", "body_fin6", "body_fin7", "body_fin2", "body_fin8"}, false);
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/body_tail.json",
{"5", "4", "7", "11", "20", "18", "19", "24", "25", "26", "27", "28", "10", "12",
    "17", "16", "15", "14", "13", "23", "22", "21", "6", "8", "9", "1", "2", "3"}, false);
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/butt_tail.json",
{"butt_tail2", "butt_tail1", "butt_tail3", "butt_tail4"}, false);
    appContext.modelSerializer->importHelper(appContext, "../res/models/outlines/body_fin_top.json",
                                {"body_fin_top"}, false);
    appContext.baseDimensions.y = 50;

//    std::reverse(appContext.outlines["wings"].begin(), appContext.outlines["wings"].end());
    std::reverse(appContext.outlines["tail5"].begin(), appContext.outlines["tail5"].end());
    std::reverse(appContext.outlines["tail6"].begin(), appContext.outlines["tail6"].end());
    std::reverse(appContext.outlines["tail3"].begin(), appContext.outlines["tail3"].end());
    std::reverse(appContext.outlines["tail4"].begin(), appContext.outlines["tail4"].end());
    std::reverse(appContext.outlines["tail7"].begin(), appContext.outlines["tail7"].end());
    std::reverse(appContext.outlines["tail8"].begin(), appContext.outlines["tail8"].end());

    std::reverse(appContext.outlines["butt1"].begin(), appContext.outlines["butt1"].end());
    std::reverse(appContext.outlines["butt2"].begin(), appContext.outlines["butt2"].end());
    std::reverse(appContext.outlines["butt3"].begin(), appContext.outlines["butt3"].end());

    std::vector<glm::vec3> bodyFin;
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin1"].begin(), appContext.outlines["body_fin1"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin2"].begin(), appContext.outlines["body_fin2"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin3"].begin(), appContext.outlines["body_fin3"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin4"].begin(), appContext.outlines["body_fin4"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin5"].begin(), appContext.outlines["body_fin5"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin6"].begin(), appContext.outlines["body_fin6"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin7"].begin(), appContext.outlines["body_fin7"].end());
    bodyFin.insert(bodyFin.end(), appContext.outlines["body_fin8"].begin(), appContext.outlines["body_fin8"].end());
    appContext.outlines["body_fin"] = bodyFin;

    std::vector<glm::vec3> buttTail;
    buttTail.insert(buttTail.end(), appContext.outlines["butt_tail1"].begin(), appContext.outlines["butt_tail1"].end());
    buttTail.insert(buttTail.end(), appContext.outlines["butt_tail2"].begin(), appContext.outlines["butt_tail2"].end());
    buttTail.insert(buttTail.end(), appContext.outlines["butt_tail3"].begin(), appContext.outlines["butt_tail3"].end());
    buttTail.insert(buttTail.end(), appContext.outlines["butt_tail4"].begin(), appContext.outlines["butt_tail4"].end());
    appContext.outlines["butt_tail"] = buttTail;

    std::vector<glm::vec3> bodyTail;
    for (int i = 1; i <= 28; ++i) {
        std::string key = std::to_string(i);
        bodyTail.insert(bodyTail.end(), appContext.outlines[key].begin(), appContext.outlines[key].end());
    }
    appContext.outlines["body_tail"] = bodyTail;

    auto &v = appContext.outlines["wings"];
    appContext.outlines["wings_top"] = std::vector<glm::vec3>(v.begin(), v.begin() + v.size() / 2);
    appContext.outlines["wings_bottom"] = std::vector<glm::vec3>(v.begin() + v.size() / 2, v.end());

    // std::reverse(appContext.outlines["fin"].begin(), appContext.outlines["fin"].end());
    auto &v1 = appContext.outlines["fin"];
    appContext.outlines["fin_top"] = std::vector<glm::vec3>(v1.begin(), v1.begin() + v1.size() / 2);
    appContext.outlines["fin_bottom"] = std::vector<glm::vec3>(v1.begin() + v1.size() / 2, v1.end());

    appContext.masks["wings"] = std::make_unique<IntersectionMask>("../res/masks/wing10.png");
    appContext.masks["body"] = std::make_unique<IntersectionMask>("../res/masks/body8.png");
    appContext.masks["bottom_eye"] = std::make_unique<IntersectionMask>("../res/masks/bottom_eye1.png");
    appContext.masks["top_eye"] = std::make_unique<IntersectionMask>("../res/masks/top_eye1.png");
    appContext.masks["nose"] = std::make_unique<IntersectionMask>("../res/masks/nose.png");
    appContext.masks["bottom_fin"] = std::make_unique<IntersectionMask>("../res/masks/bottom_fin2.png");
    appContext.masks["tail"] = std::make_unique<IntersectionMask>("../res/masks/tail8.png");
    appContext.masks["butt"] = std::make_unique<IntersectionMask>("../res/masks/butt1.png");
    appContext.masks["fin"] = std::make_unique<IntersectionMask>("../res/masks/fin.png");

    appContext.pathGenerator->render();
    appContext.modelHeightMap = appContext.pathGenerator->mTexId;
}

void Scene::update() {
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
    glBindTextureUnit(2, appContext.modelHeightMap);
    appContext.millingBaseShader->setUniform("uModelHeightMap", 2);
    appContext.millingBaseShader->setUniform("useColorMap", appContext.useColorMap);
    appContext.millingBaseShader->setUniform("errorMargin", appContext.errorMargin);
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
    if(appContext.drawMill) appContext.mill->render(*appContext.phongShader);

    appContext.phongShader->setUniform("material.albedo", glm::vec4(0.5f, 0.6f, 0.7f, 1.0f));
    glm::mat4 model = glm::identity<glm::mat4>();
    model = glm::scale(model, appContext.baseDimensions);
    model = glm::rotate(model, float(std::numbers::pi/2.f), glm::vec3(1, 0, 0));
    appContext.phongShader->setUniform("model", model);
    appContext.quad->render();

    if(appContext.drawMeshes){
        glPointSize(4);
        appContext.pointShader->use();
        appContext.pointShader->setUniform("view", appContext.camera->getViewMatrix());
        appContext.pointShader->setUniform("projection", appContext.camera->getProjectionMatrix());
        for (auto &point: appContext.points) {
            point.second.render(*appContext.pointShader);
        }
        for (auto &intersection: std::views::values(appContext.intersections)) {
            for (auto &point: intersection) {
                point.render(*appContext.pointShader);
            }
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

    }

    appContext.frameBufferManager->unbind();
}
