#pragma once

#include "Common.h"

#define WIDTH 1024
#define HEIGHT 512

using namespace std;

namespace renderer2d {
    #define step(x, start, end) start + (end - start) * x
    #define inverseStep(x, start, end) (x - start) / (end - start)
    #define interpolate(x, start, end, newStart, newEnd) step(inverseStep(x, start, end), newStart, newEnd)
    glm::vec2 convertToTex(glm::vec2 x, CanvasPoint start, CanvasPoint end) {
        //if(start.x == end.x) return glm::vec2(x.x, interpolate(x.y, start.y, end.y, start.texturePoint.y, end.texturePoint.y));
        //if(start.y == end.y) return glm::vec2(interpolate(x.x, start.x, end.x, start.texturePoint.x, end.texturePoint.x), x.y);
        return interpolate(x, start.vec2(), end.vec2(), start.texturePoint, end.texturePoint);
    }

    void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color) {
        auto diff = to - from;
        float numberOfSteps = floor(fmax(abs(diff.x), abs(diff.y)));
        auto stepSize = diff / numberOfSteps;
        for(float i = 0.0; i < numberOfSteps; i++) {
            auto point = from + stepSize * i;
            window.setPixelColour(floor(point.x), floor(point.y), encodeColor(color));
        }
    }

    void drawDepthLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, vector<vector<float>> &depthBuffer) {
        auto diff = to - from;
        float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
        auto stepSize = diff / numberOfSteps;
        for(float i = 0.0; i < numberOfSteps; i++) {
            auto point = from + stepSize * i;
            auto x = round(point.x);
            auto y = round(point.y);
            auto z = -point.depth;
            if(y > 0 && x > 0 && y < depthBuffer.size() && x < depthBuffer[y].size() && z > depthBuffer[y][x]) {
                window.setPixelColour(x, y, encodeColor(color));
                depthBuffer[y][x] = z;
            }
        }
    }

    void drawTexLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, TextureMap &map, vector<vector<float>> &depthBuffer) {
        auto diff = to - from;
        float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
        auto stepSize = diff / numberOfSteps;
        for(float i = 0.0; i < numberOfSteps; i++) {
            auto point = from + stepSize * i;
            auto x = round(point.x);
            auto y = round(point.y);
            auto z = -point.depth;
            if(y > 0 && x > 0 && y < depthBuffer.size() && x < depthBuffer[y].size() && z >= depthBuffer[y][x]) {
                window.setPixelColour(x, y, map.point(floor(point.texturePoint.x), floor(point.texturePoint.y)));
                depthBuffer[y][x] = z;
            }
        }
    }

    void drawTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
        drawLine(window, triangle.v0(), triangle.v1(), color);
        drawLine(window, triangle.v1(), triangle.v2(), color);
        drawLine(window, triangle.v2(), triangle.v0(), color);
    }

    void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color, vector<vector<float>> &depthBuffer) {
        if(triangle.v0().y < triangle.v1().y) 
            std::swap(triangle.vertices[0], triangle.vertices[1]);
        
        if(triangle.v0().y < triangle.v2().y) 
            std::swap(triangle.vertices[0], triangle.vertices[2]);
        
        if(triangle.v1().y < triangle.v2().y) 
            std::swap(triangle.vertices[1], triangle.vertices[2]);
        
        float xDiff = (triangle.v2().x - triangle.v0().x);
        float yDiff = (triangle.v2().y - triangle.v0().y);
        float step = yDiff == 0 ? 0 : xDiff / yDiff;
        float xMid = triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
        float zMid = interpolate(xMid, triangle.v0().x, triangle.v2().x, triangle.v0().depth, triangle.v2().depth);

        float start = fmin(xMid, triangle.v1().x);
        float end = fmax(xMid, triangle.v1().x);
        for(float x = start; x <= end; x ++) {
            float z = interpolate(x, xMid, triangle.v1().x, zMid, triangle.v1().depth);
            drawDepthLine(window, CanvasPoint(x, triangle.v1().y, z), triangle.v0(), color, depthBuffer);
            drawDepthLine(window, CanvasPoint(x, triangle.v1().y, z), triangle.v2(), color, depthBuffer);
        }
        // drawTriangle(window, CanvasTriangle(triangle.v0(), triangle.v1(), CanvasPoint(xMid, triangle.v1().y)), Colour(255, 255, 255));
        // drawTriangle(window, CanvasTriangle(triangle.v2(), triangle.v1(), CanvasPoint(xMid, triangle.v1().y)), Colour(255, 255, 255));
    }

    void drawSimpleTriangle(DrawingWindow &window, float step, bool bottomPart, CanvasPoint top, CanvasPoint bottom, CanvasPoint mid, Colour color, vector<vector<float>> &depthBuffer) {
        auto adiff = bottom - top;
        float aStep = adiff.y == 0 ? 0 : adiff.x / adiff.y;

        float initX = mid.x;
        if(bottomPart) initX = bottom.x;
        auto initPos = bottom;

        float len = top.y - bottom.y;

        if(bottomPart) std::swap(top, bottom);
        for(float y = 0; y < len; y++) {
            float ny = initPos.y + y;
            float from = initX + step * y;
            float to = initPos.x + aStep * y;

            float fromZ = interpolate(from, top.x, mid.x, 1/top.depth, mid.depth);
            float toZ = interpolate(to, top.x, bottom.x, 1/top.depth, 1/bottom.depth);
            drawDepthLine(window, CanvasPoint(from, ny, fromZ), CanvasPoint(to, ny, toZ), color, depthBuffer);
        }
        if(bottomPart) std::swap(top, bottom);
    }

    void drawTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color, vector<vector<float>> &depthBuffer) {
        if(triangle.v0().y < triangle.v1().y)
            std::swap(triangle.vertices[0], triangle.vertices[1]);
        
        if(triangle.v0().y < triangle.v2().y) 
            std::swap(triangle.vertices[0], triangle.vertices[2]);
        
        if(triangle.v1().y < triangle.v2().y) 
            std::swap(triangle.vertices[1], triangle.vertices[2]);

    // if(triangle.v0().x == triangle.v1().x) triangle.v0().x += 0.1;
        //if(triangle.v2().x == triangle.v1().x) triangle.v2().x += 0.1;

        auto diff = (triangle.v2() - triangle.v0());
        float step = diff.y == 0 ? 0 : diff.x / diff.y;
        float xMid = triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
        auto mid = CanvasPoint(xMid, triangle.v1().y);
        mid.depth = interpolate(triangle.v1().y, triangle.v0().y, triangle.v2().y, 1/triangle.v0().depth, 1/triangle.v2().depth);

        drawSimpleTriangle(window, step, false, triangle.v0(), triangle.v1(), mid, color, depthBuffer);
        drawSimpleTriangle(window, step, true, triangle.v1(), triangle.v2(), mid, color, depthBuffer);
    }

    void drawSimpleTexTriangle(DrawingWindow &window, float step, bool bottomPart, CanvasPoint top, CanvasPoint bottom, CanvasPoint mid, TextureMap &map, vector<vector<float>> &depthBuffer) {
        auto adiff = bottom - top;
        float aStep = adiff.y == 0 ? 0 : adiff.x / adiff.y;

        float initX = mid.x;
        if(bottomPart) initX = bottom.x;
        auto initPos = bottom;

        float len = top.y - bottom.y;

        if(bottomPart) std::swap(top, bottom);
        for(float y = 0; y < len; y++) {
            float ny = initPos.y + y;
            float from = initX + step * y;
            float to = initPos.x + aStep * y;

            glm::vec2 fromTex = convertToTex(glm::vec2(from, ny), top, mid);
            glm::vec2 toTex = convertToTex(glm::vec2(to, ny), top, bottom);
            float fromZ = interpolate(from, top.x, mid.x, 1/top.depth, mid.depth);
            float toZ = interpolate(to, top.x, bottom.x, 1/top.depth, 1/bottom.depth);
            drawTexLine(window, CanvasPoint(from, ny, fromZ, fromTex), CanvasPoint(to, ny, toZ, toTex), map, depthBuffer);
        }
        if(bottomPart) std::swap(top, bottom);
    }

    void drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap &map, vector<vector<float>> &depthBuffer) {
        if(triangle.v0().y < triangle.v1().y)
            std::swap(triangle.vertices[0], triangle.vertices[1]);
        
        if(triangle.v0().y < triangle.v2().y) 
            std::swap(triangle.vertices[0], triangle.vertices[2]);
        
        if(triangle.v1().y < triangle.v2().y) 
            std::swap(triangle.vertices[1], triangle.vertices[2]);

    // if(triangle.v0().x == triangle.v1().x) triangle.v0().x += 0.1;
        //if(triangle.v2().x == triangle.v1().x) triangle.v2().x += 0.1;

        auto diff = (triangle.v2() - triangle.v0());
        float step = diff.y == 0 ? 0 : diff.x / diff.y;
        float xMid = triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
        auto mid = CanvasPoint(xMid, triangle.v1().y, convertToTex(glm::vec2(xMid, triangle.v1().y), triangle.v2(), triangle.v0()));
        mid.depth = interpolate(xMid, triangle.v0().x, triangle.v2().x, 1/triangle.v0().depth, 1/triangle.v2().depth);

        drawSimpleTexTriangle(window, step, false, triangle.v0(), triangle.v1(), mid, map, depthBuffer);
        drawSimpleTexTriangle(window, step, true, triangle.v1(), triangle.v2(), mid, map, depthBuffer);
    }

    void drawDot(DrawingWindow &window, glm::vec2 pos, float size, Colour color) {
        for(int x = 0; x < size; x++) {
            for(int y = 0; y < size; y++) {
                if(glm::length2(glm::vec2(x, y) - size/2) <= size) {
                    window.setPixelColour(pos.x + x - size/2, pos.y + y - size/2, encodeColor(color));
                }
            }
        }
    }

}