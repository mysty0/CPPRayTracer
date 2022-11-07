#pragma once

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include "Colour.h"

glm::mat4 removeTranslation(glm::mat4 mat) {
    glm::mat4 res = mat;
    res[3] = glm::vec4(0, 0, 0, 1);
    return res;
}

glm::vec3 getPosFromMatrix(glm::mat4 matrix) {
    return glm::vec3(matrix[0][3], matrix[1][3], matrix[2][3]);
}

glm::mat3 createRotationX(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        1, 0,    0,
        0, cos,  sin,
        0, -sin, cos
    );
}

glm::mat3 createRotationY(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        cos,  0, sin,
        0,    1, 0,
        -sin, 0, cos
    );
}

glm::mat3 createRotationZ(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        cos, -sin, 0,
        sin, cos,  0,
        0,   0,    1
    );
}

glm::mat4 mat3To4(glm::mat3 mat) {
    return glm::mat4(
        mat[0][0], mat[0][1], mat[0][2], 0,
        mat[1][0], mat[1][1], mat[1][2], 0,
        mat[2][0], mat[2][1], mat[2][2], 0,
        0,         0,         0,         1
    );
} 

glm::mat4 translationMatrix(glm::vec3 offset) {
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        offset.x, offset.y, offset.z, 1
    );
}

void printMat3(glm::mat3 matrix) {
    std::cout << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << std::endl;
    std::cout << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << std::endl;
    std::cout << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << std::endl;
}

void printMat4(glm::mat4 matrix) {
    std::cout << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << " " << matrix[0][3] << std::endl;
    std::cout << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << " " << matrix[0][3] << std::endl;
    std::cout << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << " " << matrix[0][3] << std::endl;
    std::cout << matrix[3][0] << " " << matrix[3][1] << " " << matrix[3][2] << " " << matrix[3][3] << std::endl;
}