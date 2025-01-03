//
// Created by USER on 10/11/2024.
//

#ifndef PATHGENERATOR_H
#define PATHGENERATOR_H

#include <vector>

#include "../../opengl/shader/Shader.h"
struct AppContext;

class PathGenerator {
    GLuint mFBO = 0;
    GLuint mDepthId = 0;
    int resolutionX = 512;
    int resolutionY = 512;

    AppContext& appContext;

    std::vector<glm::vec3> path;

public:
    GLuint mTexId = 0;
    PathGenerator(AppContext &appContext);

    void setupPathGeneration();
    void endPathGeneration();

    void generatePathK16();
    void generatePathF10();
    void generatePathAnalyticalF10();
    void generatePathAnalyticalK08Eye();
    void generatePathAnalyticalK08();
    void generatePathAnalyticalK08Inter();
    void generatePathFullF10();
    void generatePathFullK08();

    float getMaxHeight(float x, float y, float radius, float radiusMargin, int dirX);

    void render();


    static bool outsideRange(glm::vec2 cursor) ;
};



#endif //PATHGENERATOR_H
