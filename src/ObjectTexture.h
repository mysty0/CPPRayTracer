#pragma once

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <TextureMap.h>
#include <map>

using namespace std;

struct ObjectTexture {
    glm::vec3 color{};
    TextureMap map{};

    ObjectTexture() {};
    ObjectTexture(glm::vec3 color, TextureMap map) : color(color), map(map) {};
    ObjectTexture(glm::vec3 color) : color(color) {};
    ObjectTexture(TextureMap map) : map(map) {};

    bool hasTextureMap() {
        return map.height != 0 && map.width != 0;
    }
};