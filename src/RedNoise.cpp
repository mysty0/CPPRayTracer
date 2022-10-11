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
#include <TextureMap.h>

#define WIDTH 320
#define HEIGHT 240

using namespace std;

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

CanvasPoint randomPointWithTexture(DrawingWindow &window, TextureMap& map) {
    return CanvasPoint(rand() % window.width, rand() % window.height, glm::vec2(rand() % map.width, rand() % map.height));
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color) {
    auto diff = to - from;
    float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
    auto stepSize = diff / numberOfSteps;
    for(float i = 0.0; i < numberOfSteps; i++) {
        auto point = from + stepSize * i;
        window.setPixelColour(round(point.x), round(point.y), encodeColor(color));
    }
}

void drawTexLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, TextureMap &map) {
    auto diff = to - from;
    float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
    auto stepSize = diff / numberOfSteps;
    for(float i = 0.0; i < numberOfSteps; i++) {
        auto point = from + stepSize * i;
        window.setPixelColour(round(point.x), round(point.y), map.point(floor(point.texturePoint.x), floor(point.texturePoint.y)));
    }
}

void drawTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    drawLine(window, triangle.v0(), triangle.v1(), color);
    drawLine(window, triangle.v1(), triangle.v2(), color);
    drawLine(window, triangle.v2(), triangle.v0(), color);
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour color) {
    if(triangle.v0().y < triangle.v1().y) 
        std::swap(triangle.vertices[0], triangle.vertices[1]);
    
    if(triangle.v0().y < triangle.v2().y) 
        std::swap(triangle.vertices[0], triangle.vertices[2]);
    
    if(triangle.v1().y < triangle.v2().y) 
        std::swap(triangle.vertices[1], triangle.vertices[2]);
    
    std::cout << triangle.v0() << " " << triangle.v1() << " " << triangle.v2() << std::endl;

    float xDiff = (triangle.v2().x - triangle.v0().x);
    float yDiff = (triangle.v2().y - triangle.v0().y);
    float step = yDiff == 0 ? 0 : xDiff / yDiff;
    float xMid =  triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
    //float len = abs(xMid - triangle.v1().x);
    for(float x = fmin(xMid, triangle.v1().x); x <= fmax(xMid, triangle.v1().x); x ++) {
        drawLine(window, CanvasPoint(x, triangle.v1().y), triangle.v0(), color);
        drawLine(window, CanvasPoint(x, triangle.v1().y), triangle.v2(), color);
    }
}

#define step(x, start, end) start + (end - start) * x
#define inverseStep(x, start, end) (x - start) / (end - start)
#define convertCS(x, start, end, newStart, newEnd) step(inverseStep(x, start, end), newStart, newEnd)



void drawTextureTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap &map) {
    if(triangle.v0().y < triangle.v1().y)
        std::swap(triangle.vertices[0], triangle.vertices[1]);
    
    if(triangle.v0().y < triangle.v2().y) 
        std::swap(triangle.vertices[0], triangle.vertices[2]);
    
    if(triangle.v1().y < triangle.v2().y) 
        std::swap(triangle.vertices[1], triangle.vertices[2]);

    auto diff = (triangle.v2() - triangle.v0());
    float step = diff.y == 0 ? 0 : diff.x / diff.y;
    float xMid = triangle.v0().x + step * (triangle.v1().y - triangle.v0().y);
    auto mid = CanvasPoint(xMid, triangle.v1().y, glm::vec2(step(inverseStep(xMid, triangle.v2().x, triangle.v0().x), triangle.v2().texturePoint.x, triangle.v0().texturePoint.x), triangle.v1().texturePoint.y));
    
    cout << "v0 " << triangle.v0() << " v1 " << triangle.v1() << " v2 " << triangle.v2() << endl;
    cout << xMid << " " << triangle.v2().x << " " <<  triangle.v0().x << endl;
    cout << "inv s " << inverseStep(xMid, triangle.v2().x, triangle.v0().x) << endl;
    cout << "mid " << mid << endl;
    auto adiff = triangle.v1() - triangle.v0();
    float aStep = adiff.y == 0 ? 0 : adiff.x / adiff.y;

    for(float y = 0; y <= triangle.v0().y - triangle.v1().y; y++) {
        float ny = triangle.v1().y + y;
        float from = xMid + step * y;
        float to = triangle.v1().x + aStep * y;

        auto fromTex = convertCS(glm::vec2(from, ny), mid.vec2(), triangle.v0().vec2(), mid.texturePoint, triangle.v0().texturePoint);
        auto toTex = convertCS(glm::vec2(to, ny), triangle.v1().vec2(), triangle.v0().vec2(), triangle.v1().texturePoint, triangle.v0().texturePoint);
        drawTexLine(window, CanvasPoint(from, ny, fromTex), CanvasPoint(to, ny, toTex), map);
        //window.setPixelColour(from, ny, encodeColor(Colour(0, 255, 0)));
        //window.setPixelColour(to, ny, encodeColor(Colour(255, 255, 255)));
    }

    auto bdiff = triangle.v2() - triangle.v1();
    float bStep = bdiff.y == 0 ? 0 : bdiff.x / bdiff.y;

    for(float y = 0; y <= triangle.v1().y - triangle.v2().y; y++) {
        float ny = triangle.v2().y + y;
        float from = triangle.v2().x + step * y;
        float to = triangle.v2().x + bStep * y;
        // cout << mid.vec2().x << " " << mid.vec2().y << endl;
        // cout << triangle.v0().vec2().x << " " << triangle.v0().vec2().y << endl;
        // glm::vec2 v = inverseStep(glm::vec2(from, ny), mid.vec2(), triangle.v0().vec2());
        // cout << v.x << " " << v.y << endl;

        // cout << "f " << from << endl;

        // cout << mid.vec2().x << " " << mid.vec2().y << endl;
        // cout << triangle.v0().vec2().x << " " << triangle.v0().vec2().y << endl;
        // glm::vec2 v = inverseStep(glm::vec2(from, ny), mid.vec2(), triangle.v0().vec2());
        // cout << "inv " << v.x << " " << v.y << endl;
        cout << "f " << from << " ny " << ny << endl;

        cout << "(" << mid.vec2().x << " " << mid.vec2().y << ")" << endl;
        cout << "(" << triangle.v2().vec2().x << " " << triangle.v2().vec2().y << ")" << endl;
        cout << "(" << triangle.v2().texturePoint.x << " " << triangle.v2().texturePoint.y << ")" << endl;
        cout << "(" <<  mid.texturePoint.x << " " <<  mid.texturePoint.y << ")" << endl;
        auto fromTex = convertCS(glm::vec2(from, ny), triangle.v2().vec2(), mid.vec2(), triangle.v2().texturePoint, mid.texturePoint);
        cout << "inv " << fromTex.x << " " << fromTex.y << endl;
        cout << "x " << glm::vec2(from, ny).x << " " << glm::vec2(from, ny).y << endl;

        //\\auto fromTex = convertCS(glm::vec2(from, ny), triangle.v2().vec2(), mid.vec2(), triangle.v2().texturePoint, mid.texturePoint);
        auto toTex = convertCS(glm::vec2(to, ny), triangle.v2().vec2(), triangle.v1().vec2(), triangle.v2().texturePoint, triangle.v1().texturePoint);
        drawTexLine(window, CanvasPoint(from, ny, fromTex), CanvasPoint(to, ny, toTex), map);
        //window.setPixelColour(from, ny, encodeColor(Colour(255, 0, 0)));
        //window.setPixelColour(to, ny, encodeColor(Colour(255, 255, 0)));
    }

    //drawTriangle(window, triangle, Colour(0, 255, 0));
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

TextureMap map("texture.ppm");

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
        else if (event.key.keysym.sym == SDLK_u) drawTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        else if (event.key.keysym.sym == SDLK_f) drawFilledTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        else if (event.key.keysym.sym == SDLK_g) drawTextureTriangle(window, CanvasTriangle(randomPointWithTexture(window, map), randomPointWithTexture(window, map), randomPointWithTexture(window, map)), map);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(50, 0, glm::vec2(0, 0)), CanvasPoint(0, 200, glm::vec2(100, 0)), CanvasPoint(200, 200, glm::vec2(50, 50))), map);
    drawTextureTriangle(window, CanvasTriangle(CanvasPoint(160, 10, glm::vec2(195, 5)), CanvasPoint(300, 230, glm::vec2(395, 380)), CanvasPoint(10, 150, glm::vec2(65, 300))), map);
    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(0, 0), CanvasPoint(0, 50), CanvasPoint(50, 25)), map);
    
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
