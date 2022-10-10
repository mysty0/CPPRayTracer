#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <CanvasPoint.h>
#include <Colour.h>
#include <CanvasTriangle.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int resolution) {
    std::vector<float> result(resolution);
    float step = (to - from)/(resolution-1);
    for(int i = 0; i < resolution; ++i) {
        result[i] = from + step * i;
    }
    return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int resolution) {
    std::vector<glm::vec3> result(resolution);
    auto step = (to - from) / glm::vec3(resolution-1);
    for(int i = 0; i < resolution; ++i) {
        result[i] = from + step * glm::vec3(i);
    }
    return result;
}

uint32_t encodeColor(Colour color) {
    return (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
}

glm::vec2 pointToVec(CanvasPoint point) {
    return glm::vec2(point.x, point.y);
}

Colour randomColor() {
    return Colour(rand()%255, rand()%255, rand()%255);
}

CanvasPoint randomPoint(DrawingWindow &window) {
    return CanvasPoint(rand() % window.width, rand() % window.height);
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color) {
    auto fromVec = pointToVec(from);
    auto toVec = pointToVec(to);
    auto diff = toVec - fromVec;
    float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
    auto stepSize = diff / numberOfSteps;
    for(float i = 0.0; i < numberOfSteps; i++) {
        auto point = fromVec + stepSize * i;
        window.setPixelColour(round(point.x), round(point.y), encodeColor(color));
    }
}

void drawTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    drawLine(window, triangle.v0(), triangle.v1(), color);
    drawLine(window, triangle.v1(), triangle.v2(), color);
    drawLine(window, triangle.v2(), triangle.v0(), color);
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    if(triangle.v0().y < triangle.v1().y) {
        std::swap(triangle.vertices[0], triangle.vertices[1]);
    }
    if(triangle.v0().y < triangle.v2().y) {
        std::swap(triangle.vertices[0], triangle.vertices[2]);
    }
    if(triangle.v1().y < triangle.v2().y) {
        std::swap(triangle.vertices[1], triangle.vertices[2]);
    }

    std::cout << triangle.v0() << " " << triangle.v1() << " " << triangle.v2() << std::endl;

    float xDiff = (triangle.v2().x - triangle.v0().x);
    float yDiff = (triangle.v2().y - triangle.v0().y);
    float step = yDiff == 0 ? 0 : xDiff / yDiff;
    float xMid =  triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
    float len = abs(xMid - triangle.v1().x);
    for(float x = fmin(xMid, triangle.v1().x); x <= fmax(xMid, triangle.v1().x); x ++) {
        drawLine(window, CanvasPoint(x, triangle.v1().y), triangle.v0(), color);
        drawLine(window, CanvasPoint(x, triangle.v1().y), triangle.v2(), color);
    }
}

void draw(DrawingWindow &window) {
    // window.clearPixels();
    // auto color = Colour(255, 255, 0);
    // drawLine(window, glm::vec2(0, 0), glm::vec2(100, 100), color);
    // drawLine(window, glm::vec2(100, 200), glm::vec2(0, 0), color);
    // drawLine(window, glm::vec2(0, 0), glm::vec2(100, 0), color);
    // drawLine(window, glm::vec2(0, 0), glm::vec2(0, 100), color);
    // glm::vec3 topLeft(255, 0, 0);        // red 
    // glm::vec3 topRight(0, 0, 255);       // blue 
    // glm::vec3 bottomRight(0, 255, 0);    // green 
    // glm::vec3 bottomLeft(255, 255, 0);   // yellow

    // auto left = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    // auto right = interpolateThreeElementValues(topRight, bottomRight, window.height);
    // for (size_t y = 0; y < window.height; y++) {
    //     auto row = interpolateThreeElementValues(left[y], right[y], window.width);
    //     for (size_t x = 0; x < window.width; x++) {
    //         glm::vec3 col =  row[x];
    //         uint32_t color = (255 << 24) + (int(col.r) << 16) + (int(col.g) << 8) + int(col.b);
    //         window.setPixelColour(x, y, color);
    //     }
    // }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
        else if (event.key.keysym.sym == SDLK_u) drawTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        else if (event.key.keysym.sym == SDLK_f) drawFilledTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    drawFilledTriangle(window, CanvasTriangle(CanvasPoint(200, 0), CanvasPoint(100, 25), CanvasPoint(50, 50)), Colour(255, 255, 0));
    //drawFilledTriangle(window, CanvasTriangle(CanvasPoint(0, 0), CanvasPoint(100, 0), CanvasPoint(50, 50)), Colour(255, 0, 0));
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
