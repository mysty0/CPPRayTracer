#pragma once

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <TextureMap.h>
#include <map>

using namespace std;

struct ObjectTexture {
    TextureMap* base = nullptr;
    TextureMap* roughness = nullptr;
    TextureMap* bump = nullptr;
    glm::vec3 color = glm::vec3();
    float specular = 0.0f;

    ObjectTexture() = default;
    ObjectTexture(glm::vec3 color) : color(color) {};
};