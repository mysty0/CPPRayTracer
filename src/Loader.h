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
#include "Models.h"

using namespace std;

namespace loader {
    map<string, ObjectTexture> loadMTL(string path) {
        map<string, ObjectTexture> mat;

        ifstream inputStream(path);
        string nextLine;
        string name;

        while(!inputStream.eof()) {
            std::getline(inputStream, nextLine);
            if(nextLine.size() == 0) continue;
            auto tokens = split(nextLine, ' ');
            auto ins = tokens[0];
            if(ins == "newmtl") name = tokens[1];
            if(ins == "Kd") mat[name] = ObjectTexture(Colour(stof(tokens[1]) * 255, stof(tokens[2]) * 255, stof(tokens[3]) * 255));
            if(ins == "map_Kd") mat[name] = ObjectTexture(mat[name].color, TextureMap(tokens[1]));
        }

        return mat;
    }

    vector<ModelObject> loadOBJ(string path, float scale = 1) {
        vector<ModelObject> objects;

        ifstream inputStream(path);
        string nextLine;
        vector<glm::vec3> vertices;
        vector<glm::vec2> textureMappings;
        map<string, ObjectTexture> mat;
        ObjectTexture tex;
        vector<ModelTriangle> triangles;
        string name;

        while(!inputStream.eof()) {
            std::getline(inputStream, nextLine);
            if(nextLine.size() == 0) continue;
            auto tokens = split(nextLine, ' ');
            auto ins = tokens[0];
            if(ins == "mtllib") mat = loadMTL(tokens[1]);
            else if(ins == "usemtl") tex = mat[tokens[1]];
            else if(ins == "o") {
                objects.push_back({name, tex, triangles});
                triangles.clear();
                name = tokens[1];
            }
            else if(ins == "v") vertices.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])) * scale);
            else if(ins == "vt") textureMappings.push_back(glm::vec2(stof(tokens[1]) * tex.map.width, stof(tokens[2]) * tex.map.height));
            else if(ins == "f") {
                auto x = split(tokens[1], '/');
                auto y = split(tokens[2], '/');
                auto z = split(tokens[3], '/');
                if(x[1].size() == 0)
                    triangles.push_back(ModelTriangle(vertices[stoi(x[0])-1], vertices[stoi(y[0])-1], vertices[stoi(z[0])-1], tex.color));
                else
                    triangles.push_back(ModelTriangle(vertices[stoi(x[0])-1], vertices[stoi(y[0])-1], vertices[stoi(z[0])-1], textureMappings[stoi(x[1])-1], textureMappings[stoi(y[1])-1], textureMappings[stoi(z[1])-1], tex.color));
                
            }
        }
        objects.push_back({name, tex, triangles});

        return objects;
    }
}