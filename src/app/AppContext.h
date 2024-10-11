//
// Created by faliszewskii on 16.06.24.
//

#ifndef OPENGL_TEMPLATE_APPCONTEXT_H
#define OPENGL_TEMPLATE_APPCONTEXT_H

#include <memory>
#include "../opengl/framebuffer/FrameBufferManager.h"
#include "../interface/camera/BaseCamera.h"
#include "entity/quad/Quad.h"
#include "../opengl/shader/Shader.h"
#include "entity/light/PointLight.h"
#include "entity/point/Point.h"
#include "gcode/GCodeParser.h"
#include "../opengl/mesh/PositionVertex.h"
#include "../opengl/texture/Texture.h"
#include "entity/mill/MillModel.h"
#include "mill/Mill.h"

struct AppContext {
    AppContext() = default;

    std::unique_ptr<BaseCamera> camera;
    std::unique_ptr<FrameBufferManager> frameBufferManager;

    // Shaders
    std::unique_ptr<Shader> phongShader;
    std::unique_ptr<Shader> pointShader;
    std::unique_ptr<Shader> basicShader;
    std::unique_ptr<Shader> millingBaseShader;

    // TODO --- App data goes here
    std::unique_ptr<PointLight> light;
    std::unique_ptr<Point> lightBulb;
    std::unique_ptr<Quad> quad;

    std::unique_ptr<GCodeParser> gCodeParser;

    std::unique_ptr<Mill> mill;

    std::unique_ptr<Mesh<PositionVertex>> pathModel;
    std::unique_ptr<Mesh<EmptyVertex>> base;

    std::vector<std::vector<float>> heightMapData;
    std::unique_ptr<Texture> heightMap;

    glm::vec2 heightMapSize;
    glm::vec3 baseDimensions;

    float lastFrameTime;
    bool running;

};

#endif //OPENGL_TEMPLATE_APPCONTEXT_H
