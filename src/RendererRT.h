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
    const TextureMap envirenment;

public:
    int threadNumber = 16;

    RendererRT(DrawingWindow &window, const glm::mat4 &cameraMatrix, const float &f, const glm::vec2 &windowSize, const glm::ivec2& debugPoint, const TextureMap envirenment)
        : window(window), cameraMatrix(cameraMatrix), f(f), windowSize(windowSize), debugPoint(debugPoint), envirenment(envirenment) {
        cout << "env w: " << envirenment.width << " h: " << envirenment.height << endl;
    }

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

    static glm::vec2 textureCoord(const RayTriangleIntersection& intr) {
        auto* triangle = intr.intersectedTriangle;
        return triangle->texturePoints[0] * (1 - intr.e1 - intr.e2) + intr.e1 * triangle->texturePoints[1] + intr.e2 * triangle->texturePoints[2];
    }

    glm::vec3 mapColor(const RayTriangleIntersection &intr) {
        auto tex = intr.intersectedTriangle->texture.lock();
        // return tex->base.map_or([&intr](TextureMap& base) {
        //     auto coords = textureCoord(intr);
        //     return decodeColor(base.point(coords.x * base.width, coords.y * base.height));
        // }, tex->color);
        if(tex->base.has_value()) {
            auto coords = textureCoord(intr);
            auto base = *tex->base;
            return decodeColor(base.point(coords.x * base.width, coords.y * base.height));
        }
        return glm::vec3(0);
    }

    glm::vec3 mapRoughness(RayTriangleIntersection& intr) {
        auto tex = intr.intersectedTriangle->texture.lock();
        if(tex->roughness.has_value()) {
            auto coords = textureCoord(intr);
            auto base = *tex->roughness;
            return decodeColor(base.point(coords.x * base.width, coords.y * base.height));
        }
        return glm::vec3(0);
    }

    glm::vec3 mapBump(RayTriangleIntersection& intr) {
        auto tex = intr.intersectedTriangle->texture.lock();
        return tex->bump.map_or([&](TextureMap& base) {
            auto coords = textureCoord(intr);
            return decodeColor(base.point(coords.x * base.width, coords.y * base.height));
        }, glm::vec3(0));
    }

    glm::vec3 reflect(glm::vec3 lightDir, glm::vec3 normal) {
        return glm::normalize(lightDir - 2.f * normal * (glm::dot(lightDir, normal)));
    }

    glm::vec4 reflect(glm::vec4 lightDir, glm::vec4 normal) {
        return glm::normalize(lightDir - 2.f * normal * (glm::dot(lightDir, normal)));
    }

    void renderObjectsLine(vector<ModelTriangle> &triangles, Light& light, int start, int end) {
        auto cameraRot = glm::inverse(removeTranslation(cameraMatrix));
        auto camPos = vec4To3(cameraMatrix[3]) * glm::vec3(-1,-1,-1);
        for(int y = 0; y < window.height; y++) {
            for(int x = start; x < end; x++) {
                auto ray = glm::normalize(vec4To3(cameraRot * glm::vec4(x - (int)window.width/2, -y + (int)window.height/2, -f, 1)));
                auto intr = getClosestIntersection(triangles, camPos, ray);
                if(intr.intersectedTriangle != nullptr) {
                    auto lightIntr = checkIntersection(triangles, intr.intersectionPoint, light.position - intr.intersectionPoint);
                    if(lightIntr) {
                        window.setPixelColour(x, y, encodeColor(mapColor(intr) * glm::vec3(light.ambientMin)));      
                    } else { 
                        auto tex = intr.intersectedTriangle->texture.lock();

                        auto intensity = glm::pi<float>()*glm::distance2(intr.intersectionPoint, light.position)*light.inclineFactor;
                        auto lightDir = light.position-intr.intersectionPoint;
                        auto normal = intr.intersectedTriangle->vertexNormals[1] * intr.e1 + intr.intersectedTriangle->vertexNormals[2] * intr.e2 + intr.intersectedTriangle->vertexNormals[0] * (1 - intr.e1 - intr.e2);
                        auto bump = mapBump(intr) * 0.1;
                        normal = glm::normalize(normal + glm::vec3(bump.x, bump.z, bump.y));

                        auto diffuse = glm::clamp(glm::dot(normal, lightDir), 0.f, 1.0f)*light.diffusionFactor;
                        auto refl = reflect(lightDir, normal);
                        auto specular = (light.intensity / intensity) * glm::pow(glm::clamp(glm::dot(refl, ray), 0.f, 1.0f), tex->specular == 0 ? light.specularExp : tex->specular)*light.specularFactor;

                        auto color = mapColor(intr) * max((light.intensity / intensity) * diffuse, light.ambientMin) + specular;

                        if (tex->roughness != tl::nullopt) {
                            glm::vec3 nr = ray * glm::vec3(-1, -1, -1) * 2.f + glm::vec3(0.5, 0.5, 0);
                            glm::vec3 r = glm::normalize(vec4To3(reflect(vec3To4(nr), glm::transpose(cameraMatrix) * vec3To4(normal))));
                            //glm::vec3 r = ray * glm::vec3(1, -1, -1);
                            float m = 2. * sqrt(
                                pow(r.x, 2.) +
                                pow(r.y, 2.) +
                                pow(r.z + 1., 2.)
                            );
                            auto uv = (glm::vec2(r.x, r.y) / m + .5f);
                            // cout << glm::to_string(uv) << " " << envirenment.point(uv) << " " << envirenment.height << endl;
                            auto reflectionColor = decodeColor(envirenment.point(uv.x * envirenment.width, uv.y * envirenment.height));
                            color = color + reflectionColor * glm::length(mapRoughness(intr) * 0.5);
                        }

                        window.setPixelColour(x, y, encodeColor(color));
                        

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
                    glm::vec3 r = ray * glm::vec3(-1, -1, -1)*2.f + glm::vec3(0.5, 0.5, 0);
                    float m = 2. * sqrt(
                        pow(r.x, 2.) +
                        pow(r.y, 2.) +
                        pow(r.z + 1., 2.)
                    );
                    auto uv = (glm::vec2(r.x, r.y) / m + .5f);
                    // cout << glm::to_string(uv) << " " << envirenment.point(uv) << " " << envirenment.height << endl;
                    window.setPixelColour(x, y, envirenment.point(uv.x * envirenment.width, uv.y * envirenment.height));
                    //window.setPixelColour(x, y, envirenment.point(((float)x)/window.width*envirenment.width, ((float)y) / window.height * envirenment.height));//(glm::vec2(x, y) / glm::vec2(window.width, window.height)));
                }
            }
        }
    }

    glm::vec3 triangleIntersection(const ModelTriangle& triangle, const glm::vec3 &cameraPosition, glm::vec3 rayDirection, glm::vec3 &e0, glm::vec3 &e1) {
        e0 = triangle.vertices[1] - triangle.vertices[0];
        e1 = triangle.vertices[2] - triangle.vertices[0];
        glm::vec3 sp = cameraPosition - triangle.vertices[0];
        glm::mat3 de(-rayDirection, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(de) * sp;
        if(possibleSolution.x >= 0 && possibleSolution.y >= 0 && possibleSolution.z >= 0 && possibleSolution.y + possibleSolution.z <= 1) 
            return possibleSolution;
        else return glm::vec3(-1, 0, 0);
    }

    bool checkIntersection(vector<ModelTriangle>& triangles, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        glm::vec3 e0, e1;

        for(auto const &triangle : triangles) {
            auto possibleSolution = triangleIntersection(triangle, cameraPosition, rayDirection, e0, e1);
            if(possibleSolution.x > 0.000001) return true;
        }
        
        return false;
    }

    RayTriangleIntersection getClosestIntersection(vector<ModelTriangle>& triangles, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        RayTriangleIntersection inter;
        float minDst = -1;
        glm::vec3 e0, e1;

        for(auto const &triangle : triangles) {
            auto possibleSolution = triangleIntersection(triangle, cameraPosition, rayDirection, e0, e1);
            if((minDst > possibleSolution.x || minDst < 0) && possibleSolution.x >= 0) {
                minDst = possibleSolution.x;
                inter = RayTriangleIntersection(triangle.vertices[0] + e0 * possibleSolution.y + e1 * possibleSolution.z, minDst, possibleSolution.y, possibleSolution.z, &triangle, 0);
            }
        }
        
        return inter;
    }
};