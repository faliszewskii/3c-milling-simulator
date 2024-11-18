//
// Created by USER on 16/11/2024.
//

#ifndef GCODEEXPORTER_H
#define GCODEEXPORTER_H
#include <string>
#include <vector>
#include <glm/vec3.hpp>


class GCodeExporter {
public:
    static void parse(const std::string &filePath, std::vector<glm::vec3> path);
};



#endif //GCODEEXPORTER_H
