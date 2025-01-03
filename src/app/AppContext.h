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
#include "entity/patchC0/PatchC0.h"
#include "entity/patchC2/PatchC2.h"
#include "gcode/GCodeExporter.h"
#include "heightmap/HeightMap.h"
#include "intersection/SurfaceIntersection.h"
#include "mill/Mill.h"
#include "model/serializer/Serializer.h"
#include "path/PathGenerator.h"
#include "intersectionMask/IntersectionMask.h"

struct AppContext {
    AppContext() = default;

    std::unique_ptr<BaseCamera> camera;
    std::unique_ptr<FrameBufferManager> frameBufferManager;

    // Shaders
    std::unique_ptr<Shader> phongShader;
    std::unique_ptr<Shader> pointShader;
    std::unique_ptr<Shader> basicShader;
    std::unique_ptr<Shader> millingBaseShader;
    std::unique_ptr<Shader> patchC0Shader;
    std::unique_ptr<Shader> patchC2Shader;
    std::unique_ptr<Shader> patchC0ShaderQuad;
    std::unique_ptr<Shader> patchC2ShaderQuad;

    std::unique_ptr<PointLight> light;
    std::unique_ptr<Point> lightBulb;
    std::unique_ptr<Quad> quad;

    std::unique_ptr<GCodeParser> gCodeParser;
    std::unique_ptr<GCodeExporter> gCodeExporter;

    std::unique_ptr<Mill> mill;

    std::unique_ptr<Mesh<PositionVertex>> pathModel;
    std::unique_ptr<Mesh<EmptyVertex>> base;

    std::unique_ptr<HeightMap> heightMap;

    glm::vec3 baseDimensions;

    float lastFrameTime;
    bool running = true;

    std::vector<std::string> errorMessages;

    bool drawPath;

    // Model
    std::unique_ptr<Serializer> modelSerializer;
    std::unique_ptr<SurfaceIntersection> surfaceIntersection;
    std::unique_ptr<PathGenerator> pathGenerator;

    std::map<int, Point> points;
    std::map<std::string, std::vector<Point>> intersections;

    std::unique_ptr<PatchC0> tail;
    std::unique_ptr<PatchC0> topFin;
    std::unique_ptr<PatchC0> planeXZ;

    std::unique_ptr<PatchC2> bottomEye;
    std::unique_ptr<PatchC2> bottomInner;
    std::unique_ptr<PatchC2> topInner;
    std::unique_ptr<PatchC2> topEye;
    std::unique_ptr<PatchC2> body;
    std::unique_ptr<PatchC2> butt;
    std::unique_ptr<PatchC2> nose;
    std::unique_ptr<PatchC2> wings;
    std::unique_ptr<PatchC2> bottomFin;
    std::unique_ptr<PatchC2> fin;

    bool drawMeshes;
    bool drawMill;

    std::vector<std::reference_wrapper<PatchC0>> patchesC0;

    std::vector<std::reference_wrapper<PatchC2>> patchesC2;
    glm::vec3 pathOffset;
    float pathScale;
    float pathRotation;

    glm::mat4 modelTransform;
    std::map<std::string, std::vector<glm::vec3>> outlines;

    int everyNthPathPoint;

    std::map<std::string, std::unique_ptr<IntersectionMask>> masks;
    GLuint modelHeightMap;
    bool useColorMap;
    float errorMargin;
};

#endif //OPENGL_TEMPLATE_APPCONTEXT_H
