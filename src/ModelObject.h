#pragma once

#include <CanvasTriangle.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <map>
#include "ObjectTexture.h"

using namespace std;

struct ModelObject {
    string name;
    std::shared_ptr<ObjectTexture> texture{};
    vector<ModelTriangle> triangles{};

    ModelObject() {};
    ModelObject(string name, std::shared_ptr<ObjectTexture> texture, vector<ModelTriangle> triangles) : name(name), texture(texture), triangles(triangles) {};
};