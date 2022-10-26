#include "CanvasPoint.h"

CanvasPoint::CanvasPoint() :
		texturePoint(-1, -1) {}

CanvasPoint::CanvasPoint(float xPos, float yPos) :
		x(xPos),
		y(yPos),
		depth(0.0),
		brightness(1.0),
		texturePoint(-1, -1) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness, float xTexPos, float yTexPos) :
		x(xPos),
		y(yPos),
		depth(pointDepth),
		brightness(pointBrightness),
		texturePoint(xTexPos, yTexPos) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, glm::vec2 texPoint) :
		x(xPos),
		y(yPos),
		depth(0.0),
		brightness(1.0),
		texturePoint(texPoint) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, float zPos, glm::vec2 texPoint) :
		x(xPos),
		y(yPos),
		depth(zPos),
		brightness(1.0),
		texturePoint(texPoint) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, float pointDepth) :
		x(xPos),
		y(yPos),
		depth(pointDepth),
		brightness(1.0),
		texturePoint(-1, -1) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness) :
		x(xPos),
		y(yPos),
		depth(pointDepth),
		brightness(pointBrightness),
		texturePoint(-1, -1) {}

std::ostream &operator<<(std::ostream &os, const CanvasPoint &point) {
	os << "(" << point.x << ", " << point.y << ", " << point.depth << ") " << point.brightness << "[" << point.texturePoint.x << ", "<< point.texturePoint.y << "]";
	return os;
}

#define POINT_OP(op) CanvasPoint(a.x op b.x, a.y op b.y, a.depth op b.depth, a.brightness, a.texturePoint.x op b.texturePoint.x, a.texturePoint.y op b.texturePoint.y)
#define POINT_SINGLE_OP(op) CanvasPoint(a.x op b, a.y op b, a.depth op b, a.brightness, a.texturePoint.x op b, a.texturePoint.y op b)

CanvasPoint operator+(const CanvasPoint &a, const CanvasPoint &b) {
	return POINT_OP(+);
}

CanvasPoint operator-(const CanvasPoint &a, const CanvasPoint &b) {
	return POINT_OP(-);
}

CanvasPoint operator/(const CanvasPoint &a, const CanvasPoint &b) {
	return POINT_OP(/);
}

CanvasPoint operator*(const CanvasPoint &a, const CanvasPoint &b) {
	return POINT_OP(*);
}

CanvasPoint operator*(const CanvasPoint &a, const float b) {
	return POINT_SINGLE_OP(*);
}

CanvasPoint operator/(const CanvasPoint &a, const float b) {
	return POINT_SINGLE_OP(/);
}
