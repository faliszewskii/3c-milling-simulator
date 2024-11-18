//
// Created by USER on 16/11/2024.
//

#include "GCodeExporter.h"

#include <fstream>
#include <iomanip>

void GCodeExporter::parse(const std::string &filePath, std::vector<glm::vec3> path) {
    std::ofstream outfile;
    outfile.open(filePath);
    outfile << std::fixed << std::setprecision(3);

    // N64G01X-3.376Y40.362Z44.063
    for(int line = 0; line < path.size(); line++) {
        outfile << "N" << line+1 << "G01X" << path[line].x << "Y" << -path[line].z << "Z" << path[line].y << std::endl;
    }

    outfile.close();
}
