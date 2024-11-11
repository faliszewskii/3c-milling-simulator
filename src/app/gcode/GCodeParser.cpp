//
// Created by faliszewskii on 09.10.24.
//

#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <cmath>
#include "GCodeParser.h"

std::vector<glm::vec3> GCodeParser::parse(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return {};
    }

    std::string line;
    std::regex expr("G01"); //^N[0-9]+*
    std::regex exprX("X(-?[0-9]+\\.[0-9]+)");
    std::regex exprY("Y(-?[0-9]+\\.[0-9]+)");
    std::regex exprZ("Z(-?[0-9]+\\.[0-9]+)");
    std::smatch match;
    std::vector<glm::vec3> coordinates;

    std::string prev_loc = std::setlocale(LC_NUMERIC, nullptr);
    std::setlocale(LC_NUMERIC,"C");
    float x = 0;
    float y = 0;
    float z = 0;
    while (std::getline(file, line)) {
        if (std::regex_search(line, match, expr)) {
            if(std::regex_search(line, match, exprX))
                x = std::stof(std::string(match[0].first+1, match[0].second));
            if(std::regex_search(line, match, exprY))
                y = std::stof(std::string(match[0].first+1, match[0].second));
            if(std::regex_search(line, match, exprZ))
                z = std::stof(std::string(match[0].first+1, match[0].second));

            coordinates.emplace_back(x, z, -y);
        }
    }
    std::setlocale(LC_NUMERIC, prev_loc.c_str());

    return coordinates;
}
