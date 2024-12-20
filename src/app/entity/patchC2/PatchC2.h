//
// Created by USER on 09/11/2024.
//

#ifndef PATCHC2_H
#define PATCHC2_H

#include <glm/vec4.hpp>
#include "../../../opengl/mesh/PositionVertex.h"
#include "../../../opengl/shader/Shader.h"
#include "../../../opengl/mesh/Mesh.h"
#include "../point/Point.h"

class PatchC2 {
public:
    std::string name="";
    Mesh<PositionVertex> mesh;

    int patchCountX;
    int patchCountY;
    bool wrapped;
    std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;

    PatchC2(const std::vector<PositionVertex> &vertices, const std::vector<unsigned int> &indices, int x, int y, bool wrapped) :
        mesh(vertices, indices,GL_PATCHES), patchCountX(x), patchCountY(y), wrapped(wrapped) {}

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

    glm::vec4 DeBoorCoeffs(float t, int i, int knotCount)
    {
        float knotDist = 1.0f / knotCount;

        float N[5];
        float A[4];
        float B[4];

        N[1] = 1;
        for (int j = 1; j <= 3; j++)
        {
            A[j] = knotDist * (i + j) - t;
            B[j] = t - knotDist * (i + 1 - j);
            float saved = 0;
            for (int k = 1; k <= j; k++)
            {
                float term = N[k] / (A[k] + B[j + 1 - k]);
                N[k] = saved + A[k] * term;
                saved = B[j + 1 - k] * term;
            }
            N[j + 1] = saved;
        }

        return { N[1], N[2], N[3], N[4] };
    }

    glm::vec4 DeBoorDerivs(float t, int i, int knotCount)
    {

        float N[5];
        float dN[5];
        float A[4];
        float B[4];

        N[1] = 1;
        for (int j = 1; j <= 3; j++) {
            float saved = 0.0f;
            for (int k = 1; k <= j; k++) {
                float term = N[k] / (A[k] + B[j + 1 - k]);
                float derivativeTerm = (N[k + 1] - N[k]) / (A[k] + B[j + 1 - k]);
                N[k] = saved + A[k] * term;
                saved = B[j + 1 - k] * term;
                dN[j] += derivativeTerm;
            }
            dN[j + 1] = saved;
        }

        return { dN[1], dN[2], dN[3], dN[4]};
    }

    glm::vec4 descendingAlgorithm(float t, int n)
    {
        float N01 = 1;

        float N10 = N01 * (1 - t);
        float N11 = N01 * (t - 0);

        float N2_1 = N10 * (1 - t)/2.f;
        float N20 = N10 * (t - (-1))/2.f + N11 * (2 - t)/2.f;
        float N21 = N11 * (t - 0)/2.f;

        if(n == 2) return glm::vec4(N2_1, N20, N21, 0);

        float N3_2 = N2_1 * (1 - t)/3.f;
        float N3_1 = N2_1 * (t - (-2))/3.f + N20 * (2 - t)/3.f;
        float N30 = N20 * (t - (-1))/3.f + N21 * (3 - t)/3.f;
        float N31 = N21 * (t - 0)/3.f;

        return glm::vec4(N3_2, N3_1, N30, N31);
    }


    void convertToSinglePatch(float &u, float &v, glm::vec3 coefficients[][4]) {
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

    glm::vec3 evaluate (float u, float v)
    {
        glm::vec3 p[4][4];
        convertToSinglePatch(u, v, p);

        int idxU = patchCountX + 3;
        int idxV = patchCountY + 3;
        int countU = 3 * this->rangeU() + 4;
        int countV = 3 * this->rangeV() + 4;

        float u0 = float(idxU) / countU;
        float u1 = float(idxU + 1) / countU;
        float v0 = float(idxV) / countV;
        float v1 = float(idxV + 1) / countV;

        u = u0 + (u1 - u0) * u;
        v = v0 + (v1 - v0) * v;

        glm::vec4 uN = DeBoorCoeffs(u, idxU, countU);
        glm::vec4 vN = DeBoorCoeffs(v, idxV, countV);

        glm::vec3 point = uN[0] * ( vN[0]*p[0][0] + vN[1]*p[0][1] + vN[2]*p[0][2] + vN[3]*p[0][3] )
                        + uN[1] * ( vN[0]*p[1][0] + vN[1]*p[1][1] + vN[2]*p[1][2] + vN[3]*p[1][3] )
                        + uN[2] * ( vN[0]*p[2][0] + vN[1]*p[2][1] + vN[2]*p[2][2] + vN[3]*p[2][3] )
                        + uN[3] * ( vN[0]*p[3][0] + vN[1]*p[3][1] + vN[2]*p[3][2] + vN[3]*p[3][3] );

        return point;

    }

    glm::vec3 evaluateDU(float u, float v) {
        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);
        // Tablica punktów kontrolnych w postaci 1D
        glm::vec3 uCoeffs3 = glm::vec3(descendingAlgorithm(u, 2));
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

    glm::vec3 evaluateDV(float u, float v) {
        glm::vec3 coefficients[4][4];
        convertToSinglePatch(u, v, coefficients);

        glm::vec3 vCoeffs3 = glm::vec3(descendingAlgorithm(v, 2));
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


    [[nodiscard]] glm::vec3 evaluateTool(float u, float v, float radius) {
        glm::vec3 t1 = evaluateDU(u, v);
        glm::vec3 t2 = evaluateDV(u, v);
        auto normal = glm::cross(t1, t2);
        normal = glm::normalize(normal) * radius;
//        normal = glm::normalize(glm::vec3{normal.x, 0, normal.z});
//        normal *= glm::vec3(5/5.7243,1,5/5.235);

        glm::vec3 p[4][4];
        convertToSinglePatch(u, v, p);

        int idxU = patchCountX + 3;
        int idxV = patchCountY + 3;
        int countU = 3 * this->rangeU() + 4;
        int countV = 3 * this->rangeV() + 4;

        float u0 = float(idxU) / countU;
        float u1 = float(idxU + 1) / countU;
        float v0 = float(idxV) / countV;
        float v1 = float(idxV + 1) / countV;

        u = u0 + (u1 - u0) * u;
        v = v0 + (v1 - v0) * v;

        glm::vec4 uN = DeBoorCoeffs(u, idxU, countU);
        glm::vec4 vN = DeBoorCoeffs(v, idxV, countV);

        glm::vec3 point = uN[0] * ( vN[0]*p[0][0] + vN[1]*p[0][1] + vN[2]*p[0][2] + vN[3]*p[0][3] )
                          + uN[1] * ( vN[0]*p[1][0] + vN[1]*p[1][1] + vN[2]*p[1][2] + vN[3]*p[1][3] )
                          + uN[2] * ( vN[0]*p[2][0] + vN[1]*p[2][1] + vN[2]*p[2][2] + vN[3]*p[2][3] )
                          + uN[3] * ( vN[0]*p[3][0] + vN[1]*p[3][1] + vN[2]*p[3][2] + vN[3]*p[3][3] );

        return point + normal * (name=="fin"?-1.f:1.f);
    }
};

#endif //PATCHC2_H
