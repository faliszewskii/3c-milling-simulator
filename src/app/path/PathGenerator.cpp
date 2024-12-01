//
// Created by USER on 10/11/2024.
//

#include "PathGenerator.h"

#include <algorithm>
#include <functional>

#include "../AppContext.h"
#include "../../interface/camera/TopDownCamera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
PathGenerator::PathGenerator(AppContext &appContext) : appContext(appContext) {
    glGenFramebuffers(1, &mFBO);
    glGenTextures(1, &mTexId);
    glBindTexture(GL_TEXTURE_2D, mTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolutionX, resolutionY,0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mTexId, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PathGenerator::generatePathK16() {
    render();

    float startingY = 66;
    float startingX = -87;
    float startingZ = -87;

    float baseMargin = 12;
    float endingX = 87;
    float endingZ = 87;

    float depths[] =  {35, 20};

    int dirX = 1;
    int dirZ = 1;
    float radiusK16 = 8.f;

    float heightMargin = 2.f;

    float betweenTrack = radiusK16;
    int tracks = static_cast<int>((endingZ - startingZ) / betweenTrack);
    tracks++;

    float advanceX = (endingX - startingX) / static_cast<float>(resolutionX/4);

    std::vector<glm::vec3> path;
    path.emplace_back(0, startingY, 0);

    glm::vec3 mill = glm::vec3(startingX, startingY, startingZ);
    path.emplace_back(mill);
    for(float depth : depths) {
        for(int y = 0; y < tracks; y++) {
            for(int x = 0; x < resolutionX/4; x++) {
                float heightMapValue = 0;
                int hmX = static_cast<int>((x * advanceX - baseMargin) * resolutionX / 150);
                int hmY = static_cast<int>((y * betweenTrack - baseMargin) * resolutionY / 150);
                if(dirX == -1) hmX = resolutionX - hmX;
                if(dirZ == -1) hmY = resolutionY - hmY;
                int resX = 2*radiusK16 * resolutionX / 150;
                int resY = 2*radiusK16 * resolutionY / 150;
                // if(hmX >= resX/2 && hmY >= resY/2 && hmX < resolutionX - resX && hmY < resolutionY - resY) {
                    std::vector<float> v(resX*resY);
                    glReadPixels(hmX-resX/2, hmY-resY/2, resX, resY, GL_DEPTH_COMPONENT, GL_FLOAT, v.data());
                    std::vector<float> heights;
                    std::ranges::transform(v, std::back_inserter(heights), [](float value) {
                        return value>0 && value<1? (1 - value) * (50-15) + 15 : 0;
                    });
                    heightMapValue = *std::ranges::max_element(heights);
                // }
                mill.y = heightMapValue+heightMargin > depth ? heightMapValue+heightMargin: depth;
                mill += glm::vec3(dirX * advanceX, 0, 0);
                if(heightMapValue+heightMargin > depth) {
                    path.emplace_back(mill);
                }
            }
            path.emplace_back(mill);
            mill += glm::vec3(0, 0, dirZ * betweenTrack);
            path.emplace_back(mill);
            dirX *= -1;
        }
        dirZ *= -1;
    }

    path.emplace_back(startingX, startingY, startingZ);
    path.emplace_back(0, startingY, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;
    appContext.mill->setRadius(radiusK16);
    appContext.mill->setType(Spherical);


    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float PathGenerator::getMaxHeight(float x, float y, float radius, float radiusMargin, int dirX) {
    int hmX = static_cast<int>((x+75) * resolutionX / 150);
    int hmY = static_cast<int>((y+75) * resolutionY / 150);
    // if(dirX == -1) hmX = resolutionX - hmX;
    int resX = 2*(radius+radiusMargin) * resolutionX / 150;
    int resY = 2*(radius+radiusMargin) * resolutionY / 150;

    std::vector<float> v(resX*resY);
    glReadPixels(hmX-resX/2, hmY-resY/2, resX, resY, GL_DEPTH_COMPONENT, GL_FLOAT, v.data());
    std::vector<float> heights;
    std::ranges::transform(v, std::back_inserter(heights), [](float value) {
        return value>0 && value<1? (1 - value) * (50-15) + 15 : 0;
    });
    return *std::ranges::max_element(heights);
}

void PathGenerator::generatePathF10() {
    render();

    float startingY = 66;
    float startingX = -87;
    float startingZ = -87;

    float endingX = 87;
    float endingZ = 87;

    float depth = 15;

    int dirX = 1;
    float radiusF10 = 5.f;
    float betweenTrack = radiusF10;
    float radiusMargin = 3.f;

    int tracks;
    float advanceX;

    std::vector<glm::vec3> path;
    path.emplace_back(0, startingY, 0);
    glm::vec3 mill;

    auto moveX = [](int dirX, float advanceX, int i) {
        switch(i) {
            case 0:
                return glm::vec3(dirX * advanceX, 0, 0);
            case 1:
                return glm::vec3(0, 0, -(dirX * advanceX));
            case 2:
                return glm::vec3(-(dirX * advanceX), 0, 0);
            case 3:
                return glm::vec3(0, 0, dirX * advanceX);
        }
        return glm::vec3();
    };
    auto moveY = [betweenTrack](int i) {
        switch(i) {
            case 0:
                return glm::vec3(0, 0, betweenTrack);
            case 1:
                return glm::vec3(betweenTrack, 0, 0);
            case 2:
                return glm::vec3(0, 0, -betweenTrack);
            case 3:
                return glm::vec3(-betweenTrack, 0, 0);
        }
        return glm::vec3();
    };
    auto nextAngle = [&](int i) {
        switch(i) {
            case 0:
                startingX = -87;
                startingZ = -87 + 18;
                endingX = 87;
                endingZ = 60;
                mill = glm::vec3(startingX, startingY, startingZ);
                path.emplace_back(mill);
                mill.y = depth;
                path.emplace_back(mill);
                dirX = 1;
                tracks = static_cast<int>((endingZ - startingZ) / betweenTrack);
                tracks++;
                advanceX = (endingX - startingX) / static_cast<float>(resolutionX);
                break;
            case 1:
                startingZ = -87;
                mill = glm::vec3(startingX,depth,-startingZ);
                path.emplace_back(mill);
                mill += glm::vec3(15, 0, 0);
                path.emplace_back(mill);
                endingX = 60;
                tracks = static_cast<int>((endingX - mill.x) / betweenTrack);
                tracks++;
                endingZ = 80;
                advanceX = (endingZ - startingZ) / static_cast<float>(resolutionY);
                dirX = 1;
                break;
            case 2:
                startingZ = -70;
                endingZ = 70;
                endingX = 87;
                mill = glm::vec3(87,depth,-startingZ);
                path.emplace_back(mill);
                tracks = static_cast<int>((endingZ - startingZ) / betweenTrack);
                tracks++;
                endingZ = 80;
                advanceX = (endingX - startingX) / static_cast<float>(resolutionX);
                dirX = 1;
                break;
        }
    };

    for(int angle = 0; angle < 3; angle++) {
        nextAngle(angle);
        for(int y = 0; y < tracks; y++) {
            for(int x = 0; x >= 0 && x < resolutionX; x+= dirX) {
                float heightMapValue = getMaxHeight(mill.x, mill.z, radiusF10, radiusMargin, dirX);
                if(heightMapValue > depth) {
                    path.emplace_back(mill);
                    y++;
                    mill += moveY(angle);
                    float bottomHeight = getMaxHeight(mill.x, mill.z, radiusF10, radiusMargin, dirX);
                    if(bottomHeight > depth) {
                        while(getMaxHeight(mill.x, mill.z, radiusF10, radiusMargin, dirX) > depth ) {
                            x -= dirX;
                            mill -= moveX(dirX, advanceX, angle);
                            if(x < 0 || x > resolutionX) break;
                        }
                    } else {
                        while(getMaxHeight(mill.x, mill.z, radiusF10, radiusMargin, dirX) <= depth ) {
                            x += dirX;
                            mill += moveX(dirX, advanceX, angle);
                            if(x < 0 || x > resolutionX) break;
                        }
                        x -= dirX;
                        mill -= moveX(dirX, advanceX, angle);
                    }
                    path.emplace_back(mill);
                    dirX *= -1;
                }
                mill += moveX(dirX, advanceX, angle);
                //path.emplace_back(mill);
            }
            path.emplace_back(mill);
            mill += moveY(angle);
            path.emplace_back(mill);
            dirX *= -1;
        }
    }

    mill.y = startingY;
    path.emplace_back(mill);
    path.emplace_back(0, startingY, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;

    appContext.mill->setRadius(radiusF10);
    appContext.mill->setType(Flat);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PathGenerator::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, resolutionX, resolutionY);
    glClearColor(1,1,1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    TopDownCamera camera;

    appContext.patchC0ShaderQuad->use();
    appContext.patchC0ShaderQuad->setUniform("distance", 0.f);
    appContext.patchC0ShaderQuad->setUniform("selected", false);
    appContext.patchC0ShaderQuad->setUniform("color", glm::vec4(1));
    appContext.patchC0ShaderQuad->setUniform("projection", camera.getProjectionMatrix());
    appContext.patchC0ShaderQuad->setUniform("view", camera.getViewMatrix());
    appContext.patchC0ShaderQuad->setUniform("gridCountLength", 20);
    appContext.patchC0ShaderQuad->setUniform("gridCountWidth", 20);

    appContext.tail->render(*appContext.patchC0ShaderQuad);
//    appContext.topFin->render(*appContext.patchC0ShaderQuad);

    appContext.patchC2ShaderQuad->use();
    appContext.patchC2ShaderQuad->setUniform("distance", 0.f);
    appContext.patchC2ShaderQuad->setUniform("selected", false);
    appContext.patchC2ShaderQuad->setUniform("color", glm::vec4(1));
    appContext.patchC2ShaderQuad->setUniform("projection", camera.getProjectionMatrix());
    appContext.patchC2ShaderQuad->setUniform("view", camera.getViewMatrix());
    appContext.patchC2ShaderQuad->setUniform("gridCountLength", 20);
    appContext.patchC2ShaderQuad->setUniform("gridCountWidth", 20);

    appContext.bottomEye->render(*appContext.patchC2ShaderQuad);
    appContext.bottomInner->render(*appContext.patchC2ShaderQuad);
    appContext.topInner->render(*appContext.patchC2ShaderQuad);
    appContext.topEye->render(*appContext.patchC2ShaderQuad);
    appContext.body->render(*appContext.patchC2ShaderQuad);
    appContext.butt->render(*appContext.patchC2ShaderQuad);
    appContext.nose->render(*appContext.patchC2ShaderQuad);
    appContext.wings->render(*appContext.patchC2ShaderQuad);
    appContext.bottomFin->render(*appContext.patchC2ShaderQuad);
    appContext.fin->render(*appContext.patchC2ShaderQuad);

    glEnable(GL_CULL_FACE);
}

bool doSegmentsIntersect(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2) {

    glm::vec2 p = a1;
    glm::vec2 q = b1;
    glm::vec2 r = a2 - a1;
    glm::vec2 s = b2 - b1;

    auto cp = [](glm::vec2 v, glm::vec2 w) { return v.x * w.y - v.y * w.x; };

    float t = cp(q - p, s) / cp(r, s);
    float u = cp(q - p, r) / cp(r, s);

    return t >= 0 && t <= 1 && u >= 0 && u <= 1;
}

void PathGenerator::generatePathAnalyticalF10() {
    float radiusF10 = 5.0f;



    std::vector<glm::vec3> path;
    path.emplace_back(0, 66, 0);
    path.emplace_back(-87, 66, -87);
    path.emplace_back(-87, 15, -87);

    std::vector<std::string> outlinePaths = {
            "body_top",
            "wings_top",
            "wings_bottom",
            "body_top",
            "fin_top",
            "fin_bottom",
            "body_top",
            "tail6",
            "tail7",
            "tail8",
            "tail2",
            "butt3",
            "butt2",
            // "butt1",
            // "tail1",
            "tail4",
            "tail3",
            "tail5",
            "body_bottom",
            "bottom_fin_bottom",
            "bottom_fin_top",
            "body_bottom",
            "eye_bottom",
            "nose_bottom",
            "nose_top",
            "eye_top",
    };

    int everyNth = 0;
    float epsilon = 0.5;
    bool skip = false;
    int j = 0;
    for (int i = 0; i < outlinePaths.size(); i++) {
        if(!skip) j = 0;
        skip = false;
        int n1 = appContext.outlines[outlinePaths[i]].size();
        for (; j < n1; j++) {
            auto point = appContext.outlines[outlinePaths[i]][j];
            auto nextPoint = appContext.outlines[outlinePaths[i]][(j + 1)%n1];
            if(j != n1-1 && i != outlinePaths.size()-1) {
                int n2 = appContext.outlines[outlinePaths[i+1]].size();
                for(int k = 0; k < n2-1; k++) {
                    auto o = appContext.outlines[outlinePaths[i+1]][k];
                    auto nextO = appContext.outlines[outlinePaths[i+1]][(k+1)%n2];
                    static auto get2D = [](glm::vec3 v){return glm::vec2{v.x, v.z};};
                    if(doSegmentsIntersect(get2D(point), get2D(nextPoint), get2D(o), get2D(nextO))) {
                        skip = true;
                        j = (k+1)%n2;
                        break;
                    }
                }
            }
            if(j == n1-1 && i != outlinePaths.size()-1) {
                int n2 = appContext.outlines[outlinePaths[i+1]].size();
                for(int k = 0; k < n2; k++) {
                    auto o = appContext.outlines[outlinePaths[i+1]][k];
                    if(glm::length(point - o) < epsilon) {
                        skip = true;
                        j = k+1;
                        break;
                    }
                }
            }
            if (skip) break;
            if(everyNth++ % appContext.everyNthPathPoint == 0)
                path.emplace_back(point);
        }
    }

    path.emplace_back(path.back().x, 66, path.back().z);
    path.emplace_back(0, 66, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;

    appContext.mill->setRadius(radiusF10);
    appContext.mill->setType(Flat);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;
}

void PathGenerator::generatePathAnalyticalK08Eye() {
    float radiusK08 = 4;

    std::vector<glm::vec3> path;
    path.emplace_back(0, 66, 0);

    auto &path1 = appContext.outlines["inside_bottom"];
    auto &path2 = appContext.outlines["inside_top"];

    path.emplace_back(path1[0].x, 66, path1[0].z);

    int everyNth = 0;
    float firstDepth = 45;
    float step = 10;
    for(int k = 0; k < 3; k++) {
        float halfStep = step / 2;
        for(int i = 0; i < path1.size(); i++) {
            float delta = halfStep / (path1.size() - 1);
            float depth = firstDepth - k * step - delta * i;
            if(i == 0 || i == path1.size() || everyNth++ % appContext.everyNthPathPoint == 0)
                path.emplace_back(path1[i].x, depth, path1[i].z);
        }
        for(int i = 0; i < path2.size(); i++) {
            float delta = halfStep / (path2.size() - 1);
            float depth = firstDepth - halfStep - k * step - delta * i;
            if(i == 0 || i == path2.size() || everyNth++ % appContext.everyNthPathPoint == 0)
                path.emplace_back(path2[i].x, depth, path2[i].z);

        }
    }

    std::copy(appContext.outlines["inside_bottom"].begin(), appContext.outlines["inside_bottom"].end(), std::back_inserter(path));
    std::copy(appContext.outlines["inside_top"].begin(), appContext.outlines["inside_top"].end(), std::back_inserter(path));

    path.emplace_back(path.back().x, 66, path.back().z);
    path.emplace_back(0, 66, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;

    appContext.mill->setRadius(radiusK08);
    appContext.mill->setType(Spherical);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;
}

void PathGenerator::generatePathAnalyticalK08() {
    float radiusK08 = 4;

    std::vector<glm::vec3> path;
    path.emplace_back(0, 66, 0);

    int everyNth = 0;

    std::vector<std::vector<glm::vec2>> startCursors = {
        {
            {100.f / 256, 30.f / 255},
        },{
            {130.f / 256, 70.f / 255},
            {190.f / 256, 105.f / 255},
            {90.f / 256, 30.f / 255}
        },
        {
            {100.f / 256, 20.f / 255},
            {100.f / 256, 230.f / 255},
        }, {
            {151.f / 256, 9.f / 255},
            {151.f / 256, 241.f / 255},
        },{
            {110.f / 256, 160.f / 255},
        },{
            {151.f / 256, 40.f / 255},
            {220.f / 256, 250.f / 255},
        },{
            {123.f / 256, 80.f / 255},
        },{
            {123.f / 256, 80.f / 255},
        },{
            {20.f / 256, 10.f / 256},
            {146.f / 256, 2.f / 256},
            {136.f / 256, 251.f / 256},
            {1.f / 256, 244.f / 256},
            // {184.f / 256, 85.f / 256},
            {50.f / 256, 90.f / 256},
            {193.f / 256, 252.f / 256},
        },
    };

    float uStep = 0.015;
    float vStep = 0.005;
    glm::vec2 step = {uStep, vStep};
    glm::vec2 alongDir = {1, 0};
    glm::vec2 perpDir = {0, 1};

    std::vector<std::string> names = {
             "butt", "tail", "wings", "bottom_fin", "fin", "nose", "bottom_eye", "top_eye", "body"
    };
    std::map<std::string, std::variant<PatchC2*, PatchC0*>> patches = {
            {"butt", appContext.butt.get()},
            {"tail", appContext.tail.get()},
            {"wings", appContext.wings.get()},
            {"bottom_fin", appContext.bottomFin.get()},
            {"fin", appContext.fin.get()},
            {"nose", appContext.nose.get()},
            {"bottom_eye", appContext.bottomEye.get()},
            {"top_eye", appContext.topEye.get()},
            {"body", appContext.body.get()},
    };

    for(int p = 0; p < names.size(); p++){
        IntersectionMask &mask = *appContext.masks[names[p]];
        auto wings = patches[names[p]];
        for (auto startCursor: startCursors[p]) {
            auto cursor = startCursor;
            float alongSign = 1;
            float perpSign = 1;
            glm::vec<4, unsigned char> color = mask.sample(cursor.x, cursor.y);
            for (int k = 0; k < 2; k++) {
                int kk = 0;
                int max = 500000;
                std::vector<glm::vec3> miniPath;
                while (!outsideRange(cursor) && kk++ < max) {
                    if (mask.sample(cursor.x, cursor.y) == color) {
                        auto point = std::visit(overloaded {
                            [&](auto el) {
                                float u = cursor.y * el->rangeU();
                                float v = cursor.x * el->rangeV();
                                return el->evaluateTool(u, v, -radiusK08);
                        }}, wings);
                        point.y -= radiusK08;
                        miniPath.emplace_back(point);
                        cursor += alongSign * alongDir * step;
                    } else {
                        cursor += perpSign * perpDir * step;
                        if (mask.sample(cursor.x, cursor.y) == color) {
                            while (mask.sample(cursor.x, cursor.y) == color) {
                                cursor += alongSign * alongDir * step;
                            }
                            cursor -= alongSign * alongDir * step;
                        } else {
                            auto oldCursor = cursor;
                            while (mask.sample(cursor.x, cursor.y) != color && glm::length(cursor - oldCursor) < 0.5) {
                                cursor -= alongSign * alongDir * step;
                            }
                        }
                        alongSign = -alongSign;
                    }
                }
                cursor = startCursor;
                alongSign = -1;
                perpSign = -1;
                if (!miniPath.empty()) {
                    miniPath.insert(miniPath.begin(), {miniPath.front().x, 66, miniPath.front().z});
                    miniPath.emplace_back(miniPath.back().x, 66, miniPath.back().z);
                    path.insert(path.end(), miniPath.begin(), miniPath.end());
                }
            }
        }
    }



    path.insert(path.begin(), {path.front().x, 66, path.front().z});
    path.emplace_back(path.back().x, 66, path.back().z);
    path.emplace_back(0, 66, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;

    appContext.mill->setRadius(radiusK08);
    appContext.mill->setType(Spherical);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;
}

bool PathGenerator::outsideRange(glm::vec2 cursor) {
    return (cursor.x < 0 && (cursor.y < 0 || cursor.y >= 1 )) || (cursor.x >= 1 && (cursor.y < 0 || cursor.y >= 1 ));
}


void PathGenerator::generatePathAnalyticalK08Inter() {
    float radiusK08 = 4.0f;



    std::vector<glm::vec3> path;
    path.emplace_back(0, 66, 0);

    std::vector<PatchC2*> patches {
        appContext.body.get(),
        appContext.body.get(),
        appContext.body.get(),
        appContext.butt.get(),
        appContext.body.get(),
    };

    std::vector<std::string> outlinePaths = {
            "bodyWings",
            "body_fin",
            "body_tail",
            "butt_tail",
            "body_fin_top"
    };
    int everyNth = 0;
    float epsilon = 0.5;
    bool skip = false;
    int j = 0;
    for (int i = 0; i < outlinePaths.size(); i++) {
        if(!skip) j = 0;
        skip = false;
        for (; j < appContext.outlines[outlinePaths[i]].size(); j++) {
            auto uv = appContext.outlines[outlinePaths[i]][j];
            auto point = patches[i]->evaluateTool(uv.x, uv.y, -radiusK08);
            if(j == 0) path.emplace_back(glm::vec3{point.x, 66.f, point.z});
            if(j == appContext.outlines[outlinePaths[i]].size()-1) path.emplace_back(glm::vec3{point.x, 66.f, point.z});
            if(point.y < 15 + radiusK08 + 1) continue;
            // if(i != outlinePaths.size()-1) {
            //     for(int k = 0; k < appContext.outlines[outlinePaths[i+1]].size(); k++) {
            //         auto &ouv = appContext.outlines[outlinePaths[i+1]][k];
            //         auto o = appContext.body->evaluateTool(ouv.x, ouv.y, radiusK08);
            //         if(glm::length(point - o) < epsilon) {
            //             skip = true;
            //             j = k;
            //             break;
            //         }
            //     }
            // }
            if (skip) break;
            if(everyNth++ % appContext.everyNthPathPoint == 0) {
                point.y -= radiusK08;
                path.emplace_back(point);
            }
        }
    }

    path.emplace_back(path.back().x, 66, path.back().z);
    path.emplace_back(0, 66, 0);

    appContext.mill->setPath(path);
    std::vector<PositionVertex> vertices;
    std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
    appContext.running = false;

    appContext.mill->setRadius(radiusK08);
    appContext.mill->setType(Spherical);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;
}