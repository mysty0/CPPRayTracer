#pragma once

#include "TexturePoint.h"
#include <iostream>
#include <glm/glm.hpp>

struct CanvasPoint {
	float x{};
	float y{};
	float depth{};
	float brightness{};
	glm::vec2 texturePoint{};

	CanvasPoint();
	CanvasPoint(float xPos, float yPos);
	CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness, float xTexPos, float yTexPos);
	CanvasPoint(float xPos, float yPos, glm::vec2 texPoint);
	CanvasPoint(float xPos, float yPos, float pointDepth);
	CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness);
	friend std::ostream &operator<<(std::ostream &os, const CanvasPoint &point);
	friend CanvasPoint operator+(const CanvasPoint &a, const CanvasPoint &b);
	friend CanvasPoint operator-(const CanvasPoint &a, const CanvasPoint &b);
	friend CanvasPoint operator/(const CanvasPoint &a, const CanvasPoint &b);
	friend CanvasPoint operator*(const CanvasPoint &a, const CanvasPoint &b);
	friend CanvasPoint operator*(const CanvasPoint &a, const float b);
	friend CanvasPoint operator/(const CanvasPoint &a, const float b);

	operator glm::vec2() const { return glm::vec2(this->x, this->y); }
	glm::vec2 vec2() const { return glm::vec2(this->x, this->y); }
};
