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
#include "ModelObject.h"

#ifdef _WIN32
#include <direct.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#else
#include "unistd.h"
#define cwd getcwd
#define cd chdir
#endif


using namespace std;

namespace loader {
    map<string, std::shared_ptr<ObjectTexture>> loadMTL(string path) {
        map<string, std::shared_ptr<ObjectTexture>> mat;

        ifstream inputStream(path);
        string nextLine;
        string name;

        while(inputStream.good() && !inputStream.eof()) {
            std::getline(inputStream, nextLine);
            if(nextLine.size() == 0) continue;
            auto tokens = split(nextLine, ' ');
            auto ins = tokens[0];
            if (ins == "newmtl") {
                name = tokens[1];
                mat[name] = std::make_shared<ObjectTexture>();
            }
            if(ins == "Kd") mat[name]->color = glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
            if(ins == "Ns") mat[name]->specular = stof(tokens[1]);
            if(ins == "map_Kd") mat[name]->base = TextureMap(tokens[1]);
            if(ins == "map_Bump") mat[name]->bump = TextureMap(tokens[1]);
            if(ins == "map_Ns") mat[name]->roughness = TextureMap(tokens[1]);
        }

        return mat;
    }

    vector<ModelObject> loadOBJ(string path, float scale = 1, glm::vec3 defaultColor = glm::vec3()) {
        vector<ModelObject> objects;

        ifstream inputStream(path);
        string nextLine;
        vector<glm::vec3> vertices;
        vector<glm::vec2> textureMappings;
        map<string, std::shared_ptr<ObjectTexture>> mat;
        std::shared_ptr<ObjectTexture> tex = std::make_shared<ObjectTexture>(defaultColor);
        vector<ModelTriangle> triangles;
        vector<array<int, 3>> trianglesVertexIndexes;
        string name;
        std::vector<glm::vec3> vertexNormals;
        std::vector<glm::vec3> normals;

        auto recalcVertexNormals = [&]() {
            for (int i = 0; i < triangles.size(); i++) {
                if (glm::length(triangles[i].vertexNormals[0]) > 0) continue;
                for (int v = 0; v < 3; v++) {
                    auto n = vertexNormals[trianglesVertexIndexes[i][v]];
                    triangles[i].vertexNormals[v] = glm::normalize(n / glm::length(n));
                    //cout << glm::to_string(triangles[i].vertexNormals[v]) << endl;
                }
            }
        };

        char buf[4096];
        auto oldCwd = cwd(buf, sizeof(buf));
        cd(path.substr(0, path.find_last_of("\\/")).c_str());

        while(inputStream.good() && !inputStream.eof()) {
            std::getline(inputStream, nextLine);
            if(nextLine.size() == 0) continue;
            auto tokens = split(nextLine, ' ');
            auto ins = tokens[0];
            if(ins == "mtllib") mat = loadMTL(tokens[1]);
            else if(ins == "usemtl") tex = mat[tokens[1]];
            else if(ins == "o") {
                recalcVertexNormals();
                objects.push_back({name, std::move(tex), triangles});
                triangles.clear();
                trianglesVertexIndexes.clear();
                name = tokens[1];
                tex = std::make_shared<ObjectTexture>(defaultColor);
            }
            else if(ins == "v") vertices.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])) * scale);
            else if(ins == "vn") normals.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
            else if(ins == "vt") textureMappings.push_back(glm::vec2(stof(tokens[1]), stof(tokens[2])));
            else if(ins == "f") {
                auto x = split(tokens[1], '/');
                auto y = split(tokens[2], '/');
                auto z = split(tokens[3], '/');
                ModelTriangle triang;
                int v1 = stoi(x[0]) - 1;
                int v2 = stoi(y[0]) - 1;
                int v3 = stoi(z[0]) - 1;
                if(x.size() == 1|| x[1].size() == 0)
                    triang = ModelTriangle(vertices[v1], vertices[v2], vertices[v3], tex);
                else
                    triang = ModelTriangle(
                        vertices[v1], vertices[v2], vertices[v3], 
                        textureMappings[stoi(x[1])-1], textureMappings[stoi(y[1])-1], textureMappings[stoi(z[1])-1], 
                        tex
                    );

                if (x.size() > 2 && x[2].size() != 0) {
                    triang.vertexNormals[0] = normals[stoi(x[2])-1];
                    triang.vertexNormals[1] = normals[stoi(y[2])-1];
                    triang.vertexNormals[2] = normals[stoi(z[2])-1];
                }

                triang.recalculateNormal();
                triangles.push_back(triang);
                trianglesVertexIndexes.push_back({ v1, v2, v3 });

                if (vertexNormals.size() < vertices.size()) vertexNormals.resize(vertices.size());
                vertexNormals[v1] += triang.normal;
                vertexNormals[v2] += triang.normal;
                vertexNormals[v3] += triang.normal;
            }
        }
        recalcVertexNormals();
        objects.push_back({name, tex, triangles});

        cd(oldCwd);

        return objects;
    }
}