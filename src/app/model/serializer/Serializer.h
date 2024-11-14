//
// Created by faliszewskii on 12.05.24.
//

#ifndef OPENGL_SANDBOX_SERIALIZER_H
#define OPENGL_SANDBOX_SERIALIZER_H
#include <string>
#include "Scene/Scene.h"
#include "../../entity/point/Point.h"

struct AppContext;

class Serializer {
public:
    void importModel(AppContext &appContext, const std::string &path);
    void importHelper(AppContext &appContext, const std::string &path, std::vector<std::string> outlineNames);

    MG1::Scene& getScene(const std::string &path) const;

    std::map<int, glm::vec3> loadPoints(MG1::Scene &scene) const;

    void generateTransform(AppContext &appContext, std::map<int, glm::vec3>& points) const;

    void transformPoints(const AppContext &appContext, std::map<int, glm::vec3> &map) const;

    std::map<int, int> uploadPoints(AppContext &appContext, std::map<int, glm::vec3> points) const;

    void loadPatches(AppContext &appContext, MG1::Scene &scene, std::map<int, int> &idMap) const;
};


#endif //OPENGL_SANDBOX_SERIALIZER_H
