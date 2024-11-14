//
// Created by USER on 10/11/2024.
//

#include "PathGenerator.h"

#include <algorithm>
#include <functional>

#include "../AppContext.h"
#include "../../interface/camera/TopDownCamera.h"

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

    float advanceX = (endingX - startingX) / static_cast<float>(resolutionX);

    std::vector<glm::vec3> path;
    path.emplace_back(0, startingY, 0);

    glm::vec3 mill = glm::vec3(startingX, startingY, startingZ);
    path.emplace_back(mill);
    for(float depth : depths) {
        for(int y = 0; y < tracks; y++) {
            for(int x = 0; x < resolutionX; x++) {
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
                path.emplace_back(mill);
            }
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
    float radiusMargin = 2.f;

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
                startingZ = -87 + 20;
                endingX = 87;
                endingZ = 60;
                mill = glm::vec3(startingX, startingY, startingZ);
                path.emplace_back(mill);
                mill.y = depth;
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
                path.emplace_back(mill);
            }
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
            "tail6",
            "tail7",
            "tail8",
            "tail2",
            "tail1",
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

    float epsilon = 0.5;
    bool skip = false;
    int j = 0;
    for (int i = 0; i < outlinePaths.size(); i++) {
        if(!skip) j = 0;
        skip = false;
        for (; j < appContext.outlines[outlinePaths[i]].size(); j++) {
            auto point = appContext.outlines[outlinePaths[i]][j];
            if(i != outlinePaths.size()-1) {
                for(int k = 0; k < appContext.outlines[outlinePaths[i+1]].size(); k++) {
                    auto &o = appContext.outlines[outlinePaths[i+1]][k];
                    if(glm::length(point - o) < epsilon) {
                        skip = true;
                        j = k;
                        break;
                    }
                }
            }
            if (skip) break;
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
