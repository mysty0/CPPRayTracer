#include "ModelTriangle.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, ObjectTexture* texture) :
		vertices({{v0, v1, v2}}), texturePoints(), texture(texture), normal(), vertexNormals() {}

ModelTriangle::ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec2 &v0t, const glm::vec2 &v1t, const glm::vec2 &v2t, ObjectTexture* texture) :
		vertices({{v0, v1, v2}}), texturePoints({{v0t, v1t, v2t}}), texture(texture), normal(), vertexNormals() {}

void ModelTriangle::recalculateNormal() {
	normal = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
	//normal = glm::normalize(glm::cross(vertices[2] - vertices[0], vertices[1] - vertices[0]));
}

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0].x << ", " << triangle.vertices[0].y << ", " << triangle.vertices[0].z << ")\n";
	os << "(" << triangle.vertices[1].x << ", " << triangle.vertices[1].y << ", " << triangle.vertices[1].z << ")\n";
	os << "(" << triangle.vertices[2].x << ", " << triangle.vertices[2].y << ", " << triangle.vertices[2].z << ")\n";
	auto tex = triangle.texture;
	if(tex)
		os << "color (" << tex->color.r << ", " << tex->color.g << ", " << tex->color.b << ")\n";
	return os;
}
