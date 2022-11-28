#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"
#include <memory>

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	float distanceFromCamera;
	const ModelTriangle* intersectedTriangle = nullptr;
	size_t triangleIndex;
	float e1;
	float e2;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, float e1, float e2, const ModelTriangle *triangle, size_t index);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
