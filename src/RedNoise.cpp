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
#include <ModelTriangle.h>
#include <map>

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

#define step(x, start, end) start + (end - start) * x
#define inverseStep(x, start, end) (x - start) / (end - start)
#define interpolate(x, start, end, newStart, newEnd) step(inverseStep(x, start, end), newStart, newEnd)
glm::vec2 convertToTex(glm::vec2 x, CanvasPoint start, CanvasPoint end) {
    return interpolate(x, start.vec2(), end.vec2(), start.texturePoint, end.texturePoint);
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

void drawDepthLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour color, vector<vector<float>> &depthBuffer) {
    auto diff = to - from;
    float numberOfSteps = fmax(abs(diff.x), abs(diff.y));
    auto stepSize = diff / numberOfSteps;
    for(float i = 0.0; i < numberOfSteps; i++) {
        auto point = from + stepSize * i;
        auto x = round(point.x);
        auto y = round(point.y);
        auto z = -1.0/point.depth;
        if(z > depthBuffer[y][x]) {
            window.setPixelColour(x, y, encodeColor(color));
            depthBuffer[y][x] = z;
        }
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
        float z = interpolate(x, start, end, zMid, triangle.v1().depth);
        drawDepthLine(window, CanvasPoint(x, triangle.v1().y, z), triangle.v0(), color, depthBuffer);
        drawDepthLine(window, CanvasPoint(x, triangle.v1().y, z), triangle.v2(), color, depthBuffer);
    }
}

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
    auto mid = CanvasPoint(xMid, triangle.v1().y, convertToTex(glm::vec2(xMid, triangle.v1().y), triangle.v2(), triangle.v0()));

    auto adiff = triangle.v1() - triangle.v0();
    float aStep = adiff.y == 0 ? 0 : adiff.x / adiff.y;

    for(float y = 0; y < triangle.v0().y - triangle.v1().y; y++) {
        float ny = triangle.v1().y + y;
        float from = xMid + step * y;
        float to = triangle.v1().x + aStep * y;

        auto fromTex = convertToTex(glm::vec2(from, ny), triangle.v0(), mid);
        auto toTex = convertToTex(glm::vec2(to, ny), triangle.v0(), triangle.v1());
        drawTexLine(window, CanvasPoint(from, ny, fromTex), CanvasPoint(to, ny, toTex), map);
    }

    auto bdiff = triangle.v2() - triangle.v1();
    float bStep = bdiff.y == 0 ? 0 : bdiff.x / bdiff.y;

    for(float y = 0; y < triangle.v1().y - triangle.v2().y; y++) {
        float ny = triangle.v2().y + y;
        float from = triangle.v2().x + step * y;
        float to = triangle.v2().x + bStep * y;

        auto fromTex = convertToTex(glm::vec2(from, ny), triangle.v2(), mid);
        auto toTex = convertToTex(glm::vec2(to, ny), triangle.v2(), triangle.v1());
        drawTexLine(window, CanvasPoint(from, ny, fromTex), CanvasPoint(to, ny, toTex), map);
    }
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

CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPos, glm::vec3 cameraPos, float focalLen, glm::vec2 viewportSize) {
    auto relativePos = vertexPos - cameraPos;
    glm::vec2 pos = (glm::vec2(-relativePos.x, relativePos.y) / relativePos.z) * focalLen + viewportSize / 2.0;
    return CanvasPoint(pos.x, pos.y, relativePos.z);
}

void draw(DrawingWindow &window) {
}

map<string, Colour> loadMTL(string path) {
    map<string, Colour> mat;

    ifstream inputStream(path);
	string nextLine;
    string name;

    while(!inputStream.eof()) {
        std::getline(inputStream, nextLine);
        if(nextLine.size() == 0) continue;
        auto tokens = split(nextLine, ' ');
        auto ins = tokens[0];
        if(ins == "newmtl") name = tokens[1];
        if(ins == "Kd") mat[name] = Colour(stof(tokens[1]) * 255, stof(tokens[2]) * 255, stof(tokens[3]) * 255);
    }

    return mat;
}

vector<ModelTriangle> loadOBJ(string path, float scale = 1) {
    vector<ModelTriangle> triangles;

    ifstream inputStream(path);
	string nextLine;
    vector<glm::vec3> vertices;
    map<string, Colour> mat;
    Colour color;

    while(!inputStream.eof()) {
        std::getline(inputStream, nextLine);
        if(nextLine.size() == 0) continue;
        auto tokens = split(nextLine, ' ');
        auto ins = tokens[0];
        if(ins == "mtllib") mat = loadMTL(tokens[1]);
        else if(ins == "usemtl") color = mat[tokens[1]];
        // else if(ins == "o") {
        //     vertices.clear();
        //     cout << "clear" << endl;
        // }
        else if(ins == "v") vertices.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])) * scale);
        else if(ins == "f") triangles.push_back(ModelTriangle(vertices[stoi(tokens[1])-1], vertices[stoi(tokens[2])-1], vertices[stoi(tokens[3])-1], color));
    }
    return triangles;
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
        //else if (event.key.keysym.sym == SDLK_u) drawTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        //else if (event.key.keysym.sym == SDLK_f) drawFilledTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        else if (event.key.keysym.sym == SDLK_q) exit(0);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

vector<vector<float>> initDepthBuffer(int w, int h) {
    vector<vector<float>> res(h);
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            res[y] = vector<float>(w);
        }
    }
    return res;
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

   // drawDot(window, glm::vec2(10, 10), 10, Colour(255, 255, 255));

    auto depthBuffer = initDepthBuffer(WIDTH, HEIGHT);

    auto obj = loadOBJ("cornell-box.obj", 0.25);
    glm::vec3 camPos(0, 0, 4);
    float f = 240*2;
    glm::vec2 windowSize(WIDTH, HEIGHT);
    for(auto t : obj) {
        auto p1 = (getCanvasIntersectionPoint(t.vertices[0], camPos, f, windowSize));
        auto p2 = (getCanvasIntersectionPoint(t.vertices[1], camPos, f, windowSize));
        auto p3 = (getCanvasIntersectionPoint(t.vertices[2], camPos, f, windowSize));
        drawFilledTriangle(window, CanvasTriangle(p1, p2, p3), t.colour, depthBuffer);
        // drawDot(window, p1.vec2(), 10, t.colour);
        // drawDot(window, p2.vec2(), 10, t.colour);
        // drawDot(window, p3.vec2(), 10, t.colour);
    }

    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(160, 10, glm::vec2(195, 5)), CanvasPoint(300, 230, glm::vec2(395, 380)), CanvasPoint(10, 150, glm::vec2(65, 300))), map);

    //v0 (21, 165, 0) 1[401, 212] v1 (21, 95, 0) 1[141, 6] v2 (294, 4, 0) 1[198, 305]
    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(21, 165, glm::vec2(401, 212)), CanvasPoint(21, 95, glm::vec2(141, 6)), CanvasPoint(294, 4, glm::vec2(198, 305))), map);

    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
