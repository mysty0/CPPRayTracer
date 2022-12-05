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
    ObjectTexture* texture{};
    vector<ModelTriangle> triangles{};

    ModelObject() {};
    ModelObject(string name, ObjectTexture* texture, vector<ModelTriangle> triangles) : name(name), texture(texture), triangles(triangles) {};
};

struct Model {
    vector<ModelObject> objects;

    void free() {
        for (auto& obj : objects) {
            delete obj.texture;
        }
    }
};