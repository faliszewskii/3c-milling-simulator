//
// Created by USER on 10/11/2024.
//

#ifndef PATHGENERATOR_H
#define PATHGENERATOR_H

#include "../../opengl/shader/Shader.h"
struct AppContext;

class PathGenerator {
    GLuint mFBO = 0;
    GLuint mTexId = 0;
    GLuint mDepthId = 0;
    int resolutionX = 512;
    int resolutionY = 512;

    AppContext& appContext;

public:
    PathGenerator(AppContext &appContext);

    void generatePathK16();

    float getMaxHeight(float x, float y, float radius, float radiusMargin, int dirX);

    void generatePathF10();

    void render();
};



#endif //PATHGENERATOR_H
