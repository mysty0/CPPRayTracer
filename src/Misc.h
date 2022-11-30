#pragma once

#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <CanvasPoint.h>
#include <TextureMap.h>
#include <DrawingWindow.h>

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

uint32_t encodeColor(glm::vec3 color) {
    return (255 << 24) + (int(std::min(color.r * 255, 255.0f)) << 16) + (int(std::min(color.g * 255, 255.0f)) << 8) + int(std::min(color.b * 255, 255.0f));
}

glm::vec3 decodeColor(uint32_t color) {
    return glm::vec3(color >> 16 & 0xFF, color >> 8 & 0xFF, color & 0xFF)/255.0f;
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

glm::vec4 vec3To4(glm::vec3 vec) {
    return glm::vec4(vec.x, vec.y, vec.z, 1);
}

glm::vec3 vec4To3(glm::vec4 vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}