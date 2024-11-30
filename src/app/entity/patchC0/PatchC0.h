//
// Created by faliszewskii on 06.05.24.
//

#ifndef PATCHC0_H
#define PATCHC0_H

#include <glm/vec4.hpp>
#include "../../../opengl/mesh/PositionVertex.h"
#include "../../../opengl/shader/Shader.h"
#include "../../../opengl/mesh/Mesh.h"
#include "../point/Point.h"

class PatchC0 {
public:
    Mesh<PositionVertex> mesh;

    int patchCountX;
    int patchCountY;
    bool wrapped;
    std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;

    PatchC0(const std::vector<PositionVertex> &vertices, const std::vector<unsigned int> &indices, int x, int y, bool wrapped) :
        mesh(vertices,indices,GL_PATCHES), patchCountX(x), patchCountY(y), wrapped(wrapped) {}

    void updatePoint(Point &point, int i) {
        mesh.update({point.position}, i);
    }

    void render(Shader &shader) {
        glLineWidth(2);
        shader.setUniform("patchCountWidth", patchCountX);
        shader.setUniform("patchCountLength", patchCountY);

        glPatchParameteri(GL_PATCH_VERTICES, 16);
        mesh.render(6);
        glLineWidth(1);
    }

    [[nodiscard]] float rangeU() const { return static_cast<float>(patchCountX); }
    [[nodiscard]] float rangeV() const { return static_cast<float>(patchCountY); }
    [[nodiscard]] bool wrapU() const { return wrapped; }
    [[nodiscard]] static bool wrapV() { return false; }

    void convertToSinglePatch(float &u, float &v, glm::vec3 coefficients[][4]) const {
        auto indices = mesh.getIndices().value();
        int i = static_cast<int>(u);
        int j = static_cast<int>(v);
        if(i == patchCountX) i--;
        if(j == patchCountY) j--;
        for(int k=0; k<16; k++) {
            unsigned int id = indices[ 16*(j*patchCountX + i) + k];
            glm::vec3 point = controlPoints[id].second.get().position;
            coefficients[k%4][k/4] = point;
        }
        u = u - i;
        v = v - j;
    }

    [[nodiscard]] glm::vec4 descendingAlgorithm(float t, int n) const
    {
        auto bernsteinBasis = glm::mat4(0.0f);
        bernsteinBasis[0][0] = 1.0f;

        float u = 1.0 - t;

        for (int j = 1; j <= n; j++)
        {
            bernsteinBasis[j][0] = bernsteinBasis[j - 1][0] * u;

            for (int i = 1; i <= j; i++)
            {
                bernsteinBasis[j][i] = bernsteinBasis[j - 1][i] * u + bernsteinBasis[j - 1][i - 1] * t;
            }
        }

        return {bernsteinBasis[n][0], bernsteinBasis[n][1], bernsteinBasis[n][2], bernsteinBasis[n][3]};
    }


    glm::vec3 deCasteljau2D(glm::vec3 coefficients[4][4], float u, float v) const
    {
        glm::vec4 uCoeffs = descendingAlgorithm(u, 3);
        glm::vec4 vCoeffs = descendingAlgorithm(v, 3);

        glm::vec3 result = glm::vec3(0.0, 0.0, 0.0);

        for (int u = 0; u < 4; u++)
        {
            glm::vec3 partResult = glm::vec3(0.0, 0.0, 0.0);

            partResult += coefficients[u][0] * vCoeffs[0];
            partResult += coefficients[u][1] * vCoeffs[1];
            partResult += coefficients[u][2] * vCoeffs[2];
            partResult += coefficients[u][3] * vCoeffs[3];

            result += partResult * uCoeffs[u];
        }

        return result;
    }

    static void cap(float &point, float lowerLimit, float upperLimit, bool wrap) {
        if(wrap) {
            while(point < lowerLimit) point += upperLimit - lowerLimit;
            while(point > upperLimit) point -= upperLimit - lowerLimit;
        } else {
            if(point < lowerLimit) point = lowerLimit;
            if(point > upperLimit) point = upperLimit;
        }
    }

    [[nodiscard]] glm::vec3 evaluate(float u, float v) const {
        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);

        return deCasteljau2D(coefficients, u, v);
    }

    [[nodiscard]] glm::vec3 evaluateTool(float u, float v, float radius) const {
        glm::vec3 t1 = evaluateDU(u, v);
        glm::vec3 t2 = evaluateDV(u, v);
        auto normal = glm::cross(t1, t2);
        normal = glm::normalize(normal) * radius;
        // normal = glm::normalize(glm::vec3{normal.x, 0, normal.z});
        // normal *= glm::vec3(5/5.7243,1,5/5.235);


        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);


        return deCasteljau2D(coefficients, u, v) + normal;
    }

    [[nodiscard]] glm::vec3 evaluateDU(float u, float v) const {
        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);

        // Tablica punktów kontrolnych w postaci 1D
        glm::vec3 uCoeffs3 = descendingAlgorithm(u, 2);
        glm::vec4 vCoeffs4 = descendingAlgorithm(v, 3);

        glm::vec3 partResult[4];
        glm::vec3 tangent1 = glm::vec3(0);
        for (int k = 0; k < 4; k++) {
            partResult[k] = glm::vec3(0.0, 0.0, 0.0);
            partResult[k] += coefficients[k][0] * vCoeffs4[0];
            partResult[k] += coefficients[k][1] * vCoeffs4[1];
            partResult[k] += coefficients[k][2] * vCoeffs4[2];
            partResult[k] += coefficients[k][3] * vCoeffs4[3];
        }
        tangent1 += (partResult[1] - partResult[0]) * uCoeffs3[0];
        tangent1 += (partResult[2] - partResult[1]) * uCoeffs3[1];
        tangent1 += (partResult[3] - partResult[2]) * uCoeffs3[2];
        tangent1 *= 3;

        return tangent1;
    }

    [[nodiscard]] glm::vec3 evaluateDV(float u, float v) const {
        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);
        glm::vec3 vCoeffs3 = descendingAlgorithm(v, 2);
        glm::vec4 uCoeffs4 = descendingAlgorithm(u, 3);

        glm::vec3 partResult[4];
        glm::vec3 tangent2 = glm::vec3(0);
        for (int k = 0; k < 4; k++) {
            partResult[k] = glm::vec3(0.0, 0.0, 0.0);
            partResult[k] += coefficients[0][k] * uCoeffs4[0];
            partResult[k] += coefficients[1][k] * uCoeffs4[1];
            partResult[k] += coefficients[2][k] * uCoeffs4[2];
            partResult[k] += coefficients[3][k] * uCoeffs4[3];
        }
        tangent2 += (partResult[1] - partResult[0]) * vCoeffs3[0];
        tangent2 += (partResult[2] - partResult[1]) * vCoeffs3[1];
        tangent2 += (partResult[3] - partResult[2]) * vCoeffs3[2];
        tangent2 *= 3;

        return tangent2;
    }
};

#endif //PATCHC0_H
