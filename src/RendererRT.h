#pragma once

#include "Common.h"
#include <thread>
#include <cassert>

struct Light {
    glm::vec3 position;
    float intensity;
    float diffusionFactor;
    float inclineFactor;
    float specularFactor;
    float specularExp;
    float ambientMin;
};

class RendererRT {
    DrawingWindow &window;
    const glm::mat4& cameraMatrix;
    const float& f;
    const glm::vec2& windowSize;
    const glm::ivec2& debugPoint;

public:
    int threadNumber = 16;

    RendererRT(DrawingWindow &window, const glm::mat4 &cameraMatrix, const float &f, const glm::vec2 &windowSize, const glm::ivec2& debugPoint)
        : window(window), cameraMatrix(cameraMatrix), f(f), windowSize(windowSize), debugPoint(debugPoint) {}

    void renderObjects(vector<ModelTriangle>& triangles, Light& light) {
        std::vector<std::thread> threads(threadNumber);
        int size = window.width / threadNumber;
        for (int i = 0; i < threadNumber; i++) {
            threads[i] = std::thread([this, &triangles, &light, i, size] { renderObjectsLine(triangles, light, size*i, size*i + size); });
        }

        for (int i = 0; i < threadNumber; i++) {
            threads[i].join();
        }
    }

    glm::vec3 mapColor(RayTriangleIntersection &intr) {
        auto tex = intr.intersectedTriangle->texture;
        auto base = tex->base;
        if (base) {
            auto* triangle = intr.intersectedTriangle;
            return decodeColor(base->point(triangle->texturePoints[0] * (1 - intr.e1 - intr.e2) + intr.e1 * triangle->texturePoints[1] + intr.e2 * triangle->texturePoints[2]));
        } else {
            return tex->color;
        }
    }

    struct Vertices {
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        const ModelTriangle* triangle;
    };

    void renderObjectsLine(vector<ModelTriangle> &triangles, Light& light, int start, int end) {
        auto cameraRot = glm::inverse(removeTranslation(cameraMatrix));
        auto camPos = vec4To3(cameraMatrix[3]) * glm::vec3(-1,-1,-1);
        
        auto centerRay = glm::normalize(vec4To3(cameraRot * glm::vec4(0, 0, -1, 1)));
        vector<Vertices> vertices;
        for (int i = 0; i < triangles.size(); ++i) {
            auto triangle = &triangles[i];
            vertices.push_back(Vertices{ triangle->vertices[0], triangle->vertices[1], triangle->vertices[2], triangle });
        }

        for(int y = 0; y < window.height; y++) {
            for(int x = start; x < end; x++) {
                auto ray = glm::normalize(vec4To3(cameraRot * glm::vec4(x - (int)window.width/2, -y + (int)window.height/2, -f, 1)));
                auto intr = getClosestIntersection(vertices, camPos, ray);
                if(intr.intersectedTriangle != nullptr) {
                    auto lightIntr = checkIntersection(vertices, intr.intersectionPoint, light.position - intr.intersectionPoint);
                    if(lightIntr) {
                        window.setPixelColour(x, y, encodeColor(mapColor(intr) * glm::vec3(light.ambientMin)));      
                    } else { 
                        auto intensity = glm::pi<float>()*glm::distance2(intr.intersectionPoint, light.position)*light.inclineFactor;
                        auto lightDir = light.position-intr.intersectionPoint;
                        auto normal = intr.intersectedTriangle->vertexNormals[1] * intr.e1 + intr.intersectedTriangle->vertexNormals[2] * intr.e2 + intr.intersectedTriangle->vertexNormals[0] * (1 - intr.e1 - intr.e2);
                        normal = glm::normalize(normal / glm::length(normal));
                        auto diffuse = glm::clamp(glm::dot(normal, lightDir), 0.f, 1.0f)*light.diffusionFactor;
                        auto reflect = glm::normalize(lightDir - 2.f * normal * (glm::dot(lightDir, normal)));
                        auto specular = (light.intensity / intensity) * glm::pow(glm::clamp(glm::dot(reflect, ray), 0.f, 1.0f), light.specularExp)*light.specularFactor;
                        window.setPixelColour(x, y, encodeColor(mapColor(intr) * max((light.intensity / intensity) * diffuse, light.ambientMin) + specular));

                        if (debugPoint.x == x && debugPoint.y == y) {
                            renderer2d::drawDot(window, debugPoint, 6, glm::vec3(255));
                            auto intrPos = Renderer3d::getCanvasIntersectionPoint(intr.intersectionPoint, cameraMatrix, f, windowSize);
                            auto normalPos = Renderer3d::getCanvasIntersectionPoint(intr.intersectionPoint + normal, cameraMatrix, f, windowSize);
                            auto lightPos = Renderer3d::getCanvasIntersectionPoint(intr.intersectionPoint + lightDir, cameraMatrix, f, windowSize);
                            renderer2d::drawLine(window, CanvasPoint((int)window.width / 2, (int)window.height / 2), intrPos, glm::vec3(255));
                            renderer2d::drawLine(window, normalPos, intrPos, glm::vec3(0, 255, 0));
                            renderer2d::drawLine(window, lightPos, intrPos, glm::vec3(255, 255, 0));
                        }
                    }
                } else {
                    window.setPixelColour(x, y, 0);
                }
            }
        }
    }

    glm::vec3 triangleIntersection(const Vertices& vertices, const glm::vec3 &cameraPosition, glm::vec3 rayDirection, glm::vec3 &e0, glm::vec3 &e1) {
        e0 = vertices.v2 - vertices.v1;
        e1 = vertices.v3 - vertices.v1;
        glm::vec3 sp = cameraPosition - vertices.v1;
        glm::mat3 de(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(de) * sp;
        if(possibleSolution.x >= 0 && possibleSolution.y >= 0 && possibleSolution.z >= 0 && possibleSolution.y + possibleSolution.z <= 1) 
            return possibleSolution;
        else return glm::vec3(-1, 0, 0);
    }

    bool checkIntersection(vector<Vertices> &vertices, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        glm::vec3 e0, e1;

        for(auto const & vertice : vertices) {
            auto possibleSolution = triangleIntersection(vertice, cameraPosition, rayDirection, e0, e1);
            if(possibleSolution.x > 0.000001) return true;
        }
        
        return false;
    }

    RayTriangleIntersection getClosestIntersection(vector<Vertices>& vertices, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        RayTriangleIntersection inter;
        float minDst = -1;
        glm::vec3 e0, e1;

        for(auto const &vertice : vertices) {
            auto possibleSolution = triangleIntersection(vertice, cameraPosition, rayDirection, e0, e1);
            if((minDst > possibleSolution.x || minDst < 0) && possibleSolution.x >= 0) {
                minDst = possibleSolution.x;
                inter = RayTriangleIntersection(vertice.v1 + e0 * possibleSolution.y + e1 * possibleSolution.z, minDst, possibleSolution.y, possibleSolution.z, vertice.triangle, 0);
            }
        }
        
        return inter;
    }
};