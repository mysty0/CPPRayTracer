#include "Common.h"
#include <Utils.h>
#include <fstream>
#include <vector>
#include <map>
#include "TextRenderer.h"
#include <sstream>
#include "Loader.h"
#include "Renderer3d.h"

#define WIDTH 1024
#define HEIGHT 512

using namespace std;

vector<ModelObject> objs = loader::loadOBJ("textured-cornell-box.obj", 0.25);
float f = 240*2;
bool depthView = false;
bool wireframeEnabled = false;
float depthBrightness = 1;
glm::mat4 cameraMatrix = glm::mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, -4, 1
);
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
glm::vec2 windowSize(WIDTH, HEIGHT);
Renderer3d renderer(window, cameraMatrix, f, windowSize);

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

void draw(DrawingWindow &window) {
    window.clearPixels();
    renderer.clearDepthBuffer();
    //printMat4(cameraMatrix);
    cameraMatrix[3] = cameraMatrix[3] * mat3To4(createRotationY(0.001));
    //cameraOrientation = lookAt(getPosFromMatrix(cameraMatrix), glm::vec3(0, 0, 0));
    //cout << glm::length(glm::cross(cameraOrientation[0], cameraOrientation[1]) - cameraOrientation[2]) << " " << glm::length(glm::cross(cameraOrientation[2], cameraOrientation[1]) - cameraOrientation[0]) << " " << glm::length(glm::cross(cameraOrientation[2], cameraOrientation[1]) - cameraOrientation[1]) << endl;

    renderer.renderObjects(objs);
    
    if(depthView) renderer.drawDepthBuffer(window, depthBrightness);
    if(wireframeEnabled) {
        renderer.renderWireframe(objs);
    }
}

void displayMat4(DrawingWindow& window, glm::vec2 pos, glm::mat4 matrix) {
    std::ostringstream ss("");
    ss << std::fixed << std::setprecision(3) << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << " " << matrix[0][3];
    auto r = text::renderText(window.renderer, pos, ss.str().c_str(), Colour(255, 255,255));
    ss.str(std::string());
    ss << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << " " << matrix[0][3];
    r = text::renderText(window.renderer, glm::vec2(r.x, r.y + r.h), ss.str().c_str(), Colour(255, 255,255));
    ss.str(std::string());
    ss << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << " " << matrix[0][3];
    r = text::renderText(window.renderer, glm::vec2(r.x, r.y + r.h), ss.str().c_str(), Colour(255, 255,255));
    ss.str(std::string());
    ss << matrix[3][0] << " " << matrix[3][1] << " " << matrix[3][2] << " " << matrix[3][3];
    text::renderText(window.renderer, glm::vec2(r.x, r.y + r.h), ss.str().c_str(), Colour(255, 255,255));
}

void drawOverlay(DrawingWindow& window) {
    text::renderText(window.renderer, glm::vec2(), "123", Colour(255, 255,255));
    displayMat4(window, glm::vec2(0, 30), cameraMatrix);
}

int main(int argc, char *argv[]) {
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
