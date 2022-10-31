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
#include "TextRenderer.h"

#define WIDTH 1024
#define HEIGHT 512

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
    //if(start.x == end.x) return glm::vec2(x.x, interpolate(x.y, start.y, end.y, start.texturePoint.y, end.texturePoint.y));
    //if(start.y == end.y) return glm::vec2(interpolate(x.x, start.x, end.x, start.texturePoint.x, end.texturePoint.x), x.y);
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
        auto z = -1.0/point.depth;
        //cout << z << endl;

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
        float fromZ = interpolate(from, top.x, mid.x, top.depth, mid.depth);
        float toZ = interpolate(to, top.x, bottom.x, top.depth, bottom.depth);
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
    mid.depth = interpolate(xMid, triangle.v0().x, triangle.v2().x, triangle.v0().depth, triangle.v2().depth);

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

glm::vec4 vec3To4(glm::vec3 vec) {
    return glm::vec4(vec.x, vec.y, vec.z, 1);
}

glm::mat4 removeTranslation(glm::mat4 mat) {
    glm::mat4 res = mat;
    res[3] = glm::vec4(0, 0, 0, 1);
    return res;
}

CanvasPoint getCanvasIntersectionPoint(glm::vec3 vertexPos, glm::mat4 cameraMatrix, float focalLen, glm::vec2 viewportSize) {
   // auto relativePos = removeTranslation(cameraMatrix) * (vec3To4(vertexPos) - cameraMatrix[3]);
    auto relativePos = cameraMatrix * vec3To4(vertexPos);
    glm::vec2 pos = (glm::vec2(-relativePos.x, relativePos.y) / relativePos.z) * focalLen + viewportSize / 2.0;
    return CanvasPoint(pos.x, pos.y, relativePos.z);
}

struct ObjectTexture {
    Colour color{};
    TextureMap map{};

    ObjectTexture() {};
    ObjectTexture(Colour color, TextureMap map) : color(color), map(map) {};
    ObjectTexture(Colour color) : color(color) {};
    ObjectTexture(TextureMap map) : map(map) {};
};

struct ModelObject {
    string name;
    ObjectTexture texture{};
    vector<ModelTriangle> triangles{};

    ModelObject() {};
    ModelObject(string name, ObjectTexture texture, vector<ModelTriangle> triangles) : name(name), texture(texture), triangles(triangles) {};
};

map<string, ObjectTexture> loadMTL(string path) {
    map<string, ObjectTexture> mat;

    ifstream inputStream(path);
	string nextLine;
    string name;

    while(!inputStream.eof()) {
        std::getline(inputStream, nextLine);
        if(nextLine.size() == 0) continue;
        auto tokens = split(nextLine, ' ');
        auto ins = tokens[0];
        if(ins == "newmtl") name = tokens[1];
        if(ins == "Kd") mat[name] = ObjectTexture(Colour(stof(tokens[1]) * 255, stof(tokens[2]) * 255, stof(tokens[3]) * 255));
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
    string name;

    while(!inputStream.eof()) {
        std::getline(inputStream, nextLine);
        if(nextLine.size() == 0) continue;
        auto tokens = split(nextLine, ' ');
        auto ins = tokens[0];
        if(ins == "mtllib") mat = loadMTL(tokens[1]);
        else if(ins == "usemtl") tex = mat[tokens[1]];
        else if(ins == "o") {
            objects.push_back({name, tex, triangles});
            triangles.clear();
            name = tokens[1];
        }
        else if(ins == "v") vertices.push_back(glm::vec3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3])) * scale);
        else if(ins == "vt") textureMappings.push_back(glm::vec2(stof(tokens[1]) * tex.map.width, stof(tokens[2]) * tex.map.height));
        else if(ins == "f") {
            auto x = split(tokens[1], '/');
            auto y = split(tokens[2], '/');
            auto z = split(tokens[3], '/');
            if(x[1].size() == 0)
                triangles.push_back(ModelTriangle(vertices[stoi(x[0])-1], vertices[stoi(y[0])-1], vertices[stoi(z[0])-1], tex.color));
            else
                triangles.push_back(ModelTriangle(vertices[stoi(x[0])-1], vertices[stoi(y[0])-1], vertices[stoi(z[0])-1], textureMappings[stoi(x[1])-1], textureMappings[stoi(y[1])-1], textureMappings[stoi(z[1])-1], tex.color));
            
        }
    }
    objects.push_back({name, tex, triangles});

    return objects;
}

vector<vector<float>> initDepthBuffer(int w, int h) {
    vector<vector<float>> res(h);
    for(int y = 0; y < h; y++) {
        res[y] = vector<float>(w);
    }
    return res;
}

void clearDepthBuffer(vector<vector<float>> &buffer) {
    for(int y = 0; y < buffer.size(); y++) {
        std::fill(buffer[y].begin(), buffer[y].end(), 0);
    }
}

vector<ModelObject> objs = loadOBJ("textured-cornell-box.obj", 0.25);
float f = 240*2;
bool depthView = false;
bool wireframeEnabled = false;
float depthBrightness = 1;
auto depthBuffer = initDepthBuffer(WIDTH, HEIGHT);
glm::mat4 cameraMatrix = glm::mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, -4, 1
);

glm::vec3 getPosFromMatrix(glm::mat4 matrix) {
    return glm::vec3(matrix[0][3], matrix[1][3], matrix[2][3]);
}

glm::mat3 createRotationX(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        1, 0,    0,
        0, cos,  sin,
        0, -sin, cos
    );
}

glm::mat3 createRotationY(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        cos,  0, sin,
        0,    1, 0,
        -sin, 0, cos
    );
}

glm::mat3 createRotationZ(float angle) {
    float cos = cosf(angle);
    float sin = sinf(angle);
    return glm::mat3(
        cos, -sin, 0,
        sin, cos,  0,
        0,   0,    1
    );
}

glm::mat4 mat3To4(glm::mat3 mat) {
    return glm::mat4(
        mat[0][0], mat[0][1], mat[0][2], 0,
        mat[1][0], mat[1][1], mat[1][2], 0,
        mat[2][0], mat[2][1], mat[2][2], 0,
        0,         0,         0,         1
    );
} 

glm::mat4 translationMatrix(glm::vec3 offset) {
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        offset.x, offset.y, offset.z, 1
    );
}

// glm::mat4 lookAt(glm::vec3 pos, glm::vec3 target) {
//     auto forward = glm::normalize(target - pos);
//     return glm::mat4(glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0))), glm::normalize(glm::cross(forward, glm::vec3(-1, 0, 0))), -forward);
// }

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) depthBrightness += 0.1;
        else if (event.key.keysym.sym == SDLK_DOWN) depthBrightness -= 0.1;
        else if (event.key.keysym.sym == SDLK_u) depthView = !depthView;
        else if (event.key.keysym.sym == SDLK_y) wireframeEnabled = !wireframeEnabled;
        else if (event.key.keysym.sym == SDLK_w) cameraMatrix *= translationMatrix(glm::vec3(0, 0, 0.1));
        else if (event.key.keysym.sym == SDLK_s) cameraMatrix *= translationMatrix(glm::vec3(0, 0, -0.1));
        else if (event.key.keysym.sym == SDLK_d) cameraMatrix *= translationMatrix(glm::vec3(0.1, 0, 0));
        else if (event.key.keysym.sym == SDLK_a) cameraMatrix *= translationMatrix(glm::vec3(-0.1, 0, 0));
        else if (event.key.keysym.sym == SDLK_q) cameraMatrix *= translationMatrix(glm::vec3(0, 0.1, 0));
        else if (event.key.keysym.sym == SDLK_e) cameraMatrix *= translationMatrix(glm::vec3(0, -0.1, 0));
        else if (event.key.keysym.sym == SDLK_j) cameraMatrix *= mat3To4(createRotationY(-0.1));
        else if (event.key.keysym.sym == SDLK_l) cameraMatrix *= mat3To4(createRotationY(0.1));
        else if (event.key.keysym.sym == SDLK_i) cameraMatrix *= mat3To4(createRotationX(0.1));
        else if (event.key.keysym.sym == SDLK_k) cameraMatrix *= mat3To4(createRotationX(-0.1));
        //else if (event.key.keysym.sym == SDLK_u) drawTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        //else if (event.key.keysym.sym == SDLK_f) drawFilledTriangle(window, CanvasTriangle(randomPoint(window), randomPoint(window), randomPoint(window)), randomColor());
        else if (event.key.keysym.sym == SDLK_x) exit(0);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

void drawDepthBuffer(DrawingWindow &window, vector<vector<float>> &depthBuffer) {
    for(int y = 0; y < depthBuffer.size(); y++) {
        for(int x = 0; x < depthBuffer[y].size(); x++) {
            float col = depthBuffer[y][x] * 255 * depthBrightness;
            window.setPixelColour(x, y, encodeColor(Colour(col, col, col)));
        }
    }
}

void printMat3(glm::mat3 matrix) {
    cout << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << endl;
    cout << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << endl;
    cout << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << endl;
}

void printMat4(glm::mat4 matrix) {
    cout << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << " " << matrix[0][3] << endl;
    cout << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << " " << matrix[0][3] << endl;
    cout << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << " " << matrix[0][3] << endl;
    cout << matrix[3][0] << " " << matrix[3][1] << " " << matrix[3][2] << " " << matrix[3][3] << endl;
}


void draw(DrawingWindow &window) {
    window.clearPixels();
    clearDepthBuffer(depthBuffer);
    //printMat4(cameraMatrix);
    //camPos = camPos * createRotationY(0.001);
    //cameraOrientation = lookAt(getPosFromMatrix(cameraMatrix), glm::vec3(0, 0, 0));
    //cout << glm::length(glm::cross(cameraOrientation[0], cameraOrientation[1]) - cameraOrientation[2]) << " " << glm::length(glm::cross(cameraOrientation[2], cameraOrientation[1]) - cameraOrientation[0]) << " " << glm::length(glm::cross(cameraOrientation[2], cameraOrientation[1]) - cameraOrientation[1]) << endl;

    glm::vec2 windowSize(WIDTH, HEIGHT);
    for(auto obj : objs) {
        //cout << obj.name << " " << (obj.texture.map.pixels.size()) << endl;
        for(auto t : obj.triangles) {
            auto p1 = (getCanvasIntersectionPoint(t.vertices[0], cameraMatrix, f, windowSize));
            p1.texturePoint = t.texturePoints[0];
            auto p2 = (getCanvasIntersectionPoint(t.vertices[1], cameraMatrix, f, windowSize));
            p2.texturePoint = t.texturePoints[1];
            auto p3 = (getCanvasIntersectionPoint(t.vertices[2], cameraMatrix, f, windowSize));
            p3.texturePoint = t.texturePoints[2];

            if(obj.texture.map.pixels.size() == 0) {
                drawFilledTriangle(window, CanvasTriangle(p1, p2, p3), t.colour, depthBuffer);
            } else {
                drawTextureTriangle(window, CanvasTriangle(p1, p2, p3), obj.texture.map, depthBuffer);
            }
        }
    }
    if(depthView) drawDepthBuffer(window, depthBuffer);
    if(wireframeEnabled) {
        for(auto obj : objs) {
            for(auto t : obj.triangles) {
                auto p1 = (getCanvasIntersectionPoint(t.vertices[0], cameraMatrix, f, windowSize));
                auto p2 = (getCanvasIntersectionPoint(t.vertices[1], cameraMatrix, f, windowSize));
                auto p3 = (getCanvasIntersectionPoint(t.vertices[2], cameraMatrix, f, windowSize));
                drawTriangle(window, CanvasTriangle(p1, p2, p3), Colour(255, 255, 255));
            }
        }
    }
}

void drawOverlay(DrawingWindow& window) {
    text::renderText(window.renderer, glm::vec2(), "123", Colour(255, 255,255));

}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

   // drawDot(window, glm::vec2(10, 10), 10, Colour(255, 255, 255));

    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(160, 10, glm::vec2(195, 5)), CanvasPoint(300, 230, glm::vec2(395, 380)), CanvasPoint(10, 150, glm::vec2(65, 300))), map, depthBuffer);
    
    //(261.176, 219.62, -3.2984) 1[-1, -1](90.6274, 190.08, -4.6964) 1[-1, -1](60.0602, 219.62, -3.2984) 1[-1, -1]
   // drawTextureTriangle(window, CanvasTriangle(CanvasPoint(160, 10, glm::vec2(195, 5)), CanvasPoint(300, 230, glm::vec2(395, 380)), CanvasPoint(10, 150, glm::vec2(65, 300))), map, depthBuffer);

    //v0 (21, 165, 0) 1[401, 212] v1 (21, 95, 0) 1[141, 6] v2 (294, 4, 0) 1[198, 305]
    //drawTextureTriangle(window, CanvasTriangle(CanvasPoint(21, 165, glm::vec2(401, 212)), CanvasPoint(21, 95, glm::vec2(401, 6)), CanvasPoint(294, 4, glm::vec2(198, 305))), map, depthBuffer);
    
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
        drawOverlay(window);
        window.finishRender();
    }
}
