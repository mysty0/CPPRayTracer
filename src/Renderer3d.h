#pragma once

#include "Renderer2d.h"
#include "Common.h"

class Renderer3d {
    DrawingWindow &window;
    const glm::mat4& cameraMatrix;
    const float& f;
    const glm::vec2& windowSize;
    vector<vector<float>> depthBuffer;

    private:
    vector<vector<float>> initDepthBuffer(int w, int h) {
        vector<vector<float>> res(h);
        for(int y = 0; y < h; y++) {
            res[y] = vector<float>(w);
        }
        return res;
    }

    public:
    Renderer3d(DrawingWindow &window, const glm::mat4 &cameraMatrix, const float &f, const glm::vec2 &windowSize) 
        : window(window), cameraMatrix(cameraMatrix), f(f), windowSize(windowSize) {
        depthBuffer = initDepthBuffer(windowSize.x, windowSize.y);
    }

    static CanvasPoint getCanvasIntersectionPoint(const glm::vec3 &vertexPos, const glm::mat4 &cameraMatrix, float focalLen, const glm::vec2 &viewportSize) {
        auto relativePos = removeTranslation(cameraMatrix) * (vec3To4(vertexPos) + cameraMatrix[3]);
        glm::vec2 pos = (glm::vec2(-relativePos.x, relativePos.y) / relativePos.z) * focalLen + viewportSize / 2.0;
        return CanvasPoint(pos.x, pos.y, relativePos.z);
    }

    void renderObjects(vector<ModelObject> &objs) {
        for(auto const &obj : objs) {
            //cout << obj.name << " " << (obj.texture.map.pixels.size()) << endl;
            for(auto const &t : obj.triangles) {
                auto p1 = (getCanvasIntersectionPoint(t.vertices[0], cameraMatrix, f, windowSize));
                p1.texturePoint = t.texturePoints[0];
                auto p2 = (getCanvasIntersectionPoint(t.vertices[1], cameraMatrix, f, windowSize));
                p2.texturePoint = t.texturePoints[1];
                auto p3 = (getCanvasIntersectionPoint(t.vertices[2], cameraMatrix, f, windowSize));
                p3.texturePoint = t.texturePoints[2];

                auto tex = obj.texture->base;
                if(tex) {
                    renderer2d::drawTextureTriangle(window, CanvasTriangle(p1, p2, p3), tex, depthBuffer);
                } else {
                    renderer2d::drawTriangle(window, CanvasTriangle(p1, p2, p3), obj.texture->color, depthBuffer);
                };
            }
        }
    }

    void renderWireframe(vector<ModelObject> &objs) {
        for(auto const & obj : objs) {
            for(auto const & t : obj.triangles) {
                auto p1 = (getCanvasIntersectionPoint(t.vertices[0], cameraMatrix, f, windowSize));
                auto p2 = (getCanvasIntersectionPoint(t.vertices[1], cameraMatrix, f, windowSize));
                auto p3 = (getCanvasIntersectionPoint(t.vertices[2], cameraMatrix, f, windowSize));
                renderer2d::drawTriangle(window, CanvasTriangle(p1, p2, p3), glm::vec3(1));
            }
        }
    }

    void drawDepthBuffer(DrawingWindow &window, float depthBrightness) {
        for(int y = 0; y < depthBuffer.size(); y++) {
            for(int x = 0; x < depthBuffer[y].size(); x++) {
                float col = depthBuffer[y][x] * 255 * depthBrightness;
                window.setPixelColour(x, y, encodeColor(Colour(col, col, col)));
            }
        }
    }

    void clearDepthBuffer() {
        for(int y = 0; y < depthBuffer.size(); y++) {
            std::fill(depthBuffer[y].begin(), depthBuffer[y].end(), 0);
        }
    }
};