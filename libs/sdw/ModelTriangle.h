#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "glm/ext.hpp"

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	std::array<glm::vec3, 3> vertexNormals{};
	std::array<glm::vec2, 3> texturePoints{};
	glm::vec3 colour{};
	glm::vec3 normal{};

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, glm::vec3 trigColour);
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec2 &v0t, const glm::vec2 &v1t, const glm::vec2 &v2t, glm::vec3 trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);

	void recalculateNormal();
	void recalculateVertexNormals();
};
