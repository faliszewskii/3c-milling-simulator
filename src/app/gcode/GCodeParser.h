//
// Created by faliszewskii on 09.10.24.
//

#ifndef MILLING_SIMULATOR_GCODEPARSER_H
#define MILLING_SIMULATOR_GCODEPARSER_H


#include <vector>
#include <glm/vec3.hpp>
#include <string>

class GCodeParser {
public:
    static std::vector<glm::vec3> parse(const std::string &filePath);
};


#endif //MILLING_SIMULATOR_GCODEPARSER_H
