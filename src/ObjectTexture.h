#pragma once

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <TextureMap.h>
#include <map>
#include <optional.h>

using namespace std;

struct ObjectTexture {
    tl::optional<TextureMap> base = tl::nullopt;
    tl::optional<TextureMap> roughness = tl::nullopt;
    tl::optional<TextureMap> bump = tl::nullopt;
    glm::vec3 color = glm::vec3();
    float specular = 0.0f;

    ObjectTexture() = default;
    ObjectTexture(glm::vec3 color) : color(color) {};
};