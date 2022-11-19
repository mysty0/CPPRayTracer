#pragma once

#include "Common.h"

struct Light {
    glm::vec3 position;
    float intensity;
    float diffusionFactor;
    float inclineFactor;
    float specularFactor;
};

class RendererRT {
    DrawingWindow &window;
    const glm::mat4& cameraMatrix;
    const float& f;
    const glm::vec2& windowSize;

    public:
    RendererRT(DrawingWindow &window, const glm::mat4 &cameraMatrix, const float &f, const glm::vec2 &windowSize) 
        : window(window), cameraMatrix(cameraMatrix), f(f), windowSize(windowSize) {}

    void renderObjects(vector<ModelObject> &objs, Light light) {
        auto cameraRot = removeTranslation(cameraMatrix);
        auto camPos = vec4To3(cameraMatrix[3])*glm::vec3(1,1,-1);
        for(int y = 0; y < window.height; y++) {
            for(int x = 0; x < window.width; x++) {
               // cout << x - window.width/2 << " " << y - window.height/2 << endl;
                auto ray = glm::normalize(vec4To3(cameraRot * glm::vec4(x - (int)window.width/2, y - (int)window.height/2, -f, 1)));
                auto intr = getClosestIntersection(objs, camPos, ray);
                if(intr.intersectedTriangle != nullptr) {
                    auto lightIntr = checkIntersection(objs, intr.intersectionPoint, light.position - intr.intersectionPoint);
                    if(false) {
                        window.setPixelColour(x, window.height-y-1, 0);
                    } else { 
                        auto intensity = glm::pi<float>()*glm::distance2(intr.intersectionPoint, light.position)*light.inclineFactor;
                        auto lightDir = -light.position-intr.intersectionPoint;
                        auto normal = intr.intersectedTriangle->normal;
                        auto diffuse = glm::clamp(glm::dot(normal, -lightDir), 0.f, 1.0f)*light.diffusionFactor;
                        auto reflect = glm::normalize(lightDir - 2.f * normal * (glm::dot(lightDir, normal)));
                        auto specular = glm::pow(glm::clamp(glm::dot(reflect, ray), 0.f, 1.0f), 256.f)*light.specularFactor;
                        //cout << glm::to_string(light) << " " << glm::to_string(intr->intersectedTriangle.normal) << " " << glm::dot(light, intr->intersectedTriangle.normal) << endl;
                        window.setPixelColour(x, window.height-y-1, encodeColor(intr.intersectedTriangle->colour * (light.intensity/intensity) + diffuse + specular));
                    }

                } else {
                    window.setPixelColour(x, window.height-y-1, 0);
                }
            }
        }
        auto lightPos = Renderer3d::getCanvasIntersectionPoint(light.position, cameraMatrix, f, windowSize);
        renderer2d::drawDot(window, lightPos.vec2(), 5, glm::vec3(255));
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

    bool checkIntersection(vector<ModelObject> &objs, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        glm::vec3 e0, e1;

        for(auto const &obj : objs) {
            for(auto const &triangle : obj.triangles) {
                auto possibleSolution = triangleIntersection(triangle, cameraPosition, rayDirection, e0, e1);
                if(possibleSolution.x > 0.000001) return true;
            }
        }
        return false;
    }

    RayTriangleIntersection getClosestIntersection(vector<ModelObject> &objs, glm::vec3 &cameraPosition, glm::vec3 rayDirection) {
        RayTriangleIntersection inter;
        float minDst = -1;
        glm::vec3 e0, e1;

        for(auto const &obj : objs) {
            for(auto const &triangle : obj.triangles) {
                auto possibleSolution = triangleIntersection(triangle, cameraPosition, rayDirection, e0, e1);
                if((minDst > possibleSolution.x || minDst < 0) && possibleSolution.x >= 0) {
                    minDst = possibleSolution.x;
                    inter = RayTriangleIntersection(triangle.vertices[0] + e0 * possibleSolution.y + e1 * possibleSolution.z, minDst, &triangle, 0);                    
                }
            }
        }
        return inter;
    }
};