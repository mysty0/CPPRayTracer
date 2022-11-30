#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "glm/ext.hpp"
#include "../../src/ObjectTexture.h"

struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	std::array<glm::vec3, 3> vertexNormals{};
	std::array<glm::vec2, 3> texturePoints{};
	std::weak_ptr<ObjectTexture> texture{};
	glm::vec3 normal{};

	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, std::weak_ptr<ObjectTexture> texture);
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec2 &v0t, const glm::vec2 &v1t, const glm::vec2 &v2t, std::weak_ptr<ObjectTexture> texture);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);

	void recalculateNormal();
};
