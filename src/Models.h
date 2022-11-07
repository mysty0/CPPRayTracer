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

using namespace std;

struct ObjectTexture {
    Colour color{};
    TextureMap map{};

    ObjectTexture() {};
    ObjectTexture(Colour color, TextureMap map) : color(color), map(map) {};
    ObjectTexture(Colour color) : color(color) {};
    ObjectTexture(TextureMap map) : map(map) {};
};

struct ModelObject {
    string name;
    ObjectTexture texture{};
    vector<ModelTriangle> triangles{};

    ModelObject() {};
    ModelObject(string name, ObjectTexture texture, vector<ModelTriangle> triangles) : name(name), texture(texture), triangles(triangles) {};
};