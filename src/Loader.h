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
    map<string, ObjectTexture> loadMTL(string path) {
        map<string, ObjectTexture> mat;

        ifstream inputStream(path);
        string nextLine;
        string name;

        while(inputStream.good() && !inputStream.eof()) {
            std::getline(inputStream, nextLine);
            if(nextLine.size() == 0) continue;
            auto tokens = split(nextLine, ' ');
            auto ins = tokens[0];
            if(ins == "newmtl") name = tokens[1];
            if(ins == "Kd") mat[name] = ObjectTexture(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])));
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
        vector<array<int, 3>> trianglesVertexIndexes;
        string name;
        std::vector<glm::vec3> vertexNormals;

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
                for (int i = 0; i < triangles.size(); i++) {
                    triangles[i].vertexNormals[0] = vertexNormals[trianglesVertexIndexes[i][0]];
                    triangles[i].vertexNormals[1] = vertexNormals[trianglesVertexIndexes[i][1]];
                    triangles[i].vertexNormals[2] = vertexNormals[trianglesVertexIndexes[i][2]];
                }

                objects.push_back({name, tex, triangles});
                triangles.clear();
                trianglesVertexIndexes.clear();
                name = tokens[1];
            }
            else if(ins == "v") vertices.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])) * scale);
            else if(ins == "vt") textureMappings.push_back(glm::vec2(stof(tokens[1]) * tex.map.width, stof(tokens[2]) * tex.map.height));
            else if(ins == "f") {
                auto x = split(tokens[1], '/');
                auto y = split(tokens[2], '/');
                auto z = split(tokens[3], '/');
                ModelTriangle triang;
                int v1 = stoi(x[0]) - 1;
                int v2 = stoi(y[0]) - 1;
                int v3 = stoi(z[0]) - 1;
                if(x[1].size() == 0)
                    triang = ModelTriangle(vertices[v1], vertices[v2], vertices[v3], tex.color);
                else
                    triang = ModelTriangle(
                        vertices[v1], vertices[v2], vertices[v3], 
                        textureMappings[stoi(x[1])-1], textureMappings[stoi(y[1])-1], textureMappings[stoi(z[1])-1], 
                        tex.color
                    );

                triang.recalculateNormal();
                triangles.push_back(triang);
                trianglesVertexIndexes.push_back({ v1, v2, v3 });

                if (vertexNormals.size() < vertices.size()) vertexNormals.resize(vertices.size());
                vertexNormals[v1] += triang.normal;
                vertexNormals[v2] += triang.normal;
                vertexNormals[v3] += triang.normal;
            }
        }
        objects.push_back({name, tex, triangles});

        cd(oldCwd);

        return objects;
    }
}