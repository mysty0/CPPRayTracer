#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Utils.h"
#include <glm/glm.hpp>

class TextureMap {
public:
	size_t width = 0;
	size_t height = 0;
	std::vector<uint32_t> pixels;

	TextureMap();
	TextureMap(const std::string &filename);
	uint32_t point(size_t x, size_t y) const;
	uint32_t point(glm::vec2 pos) const;
	friend std::ostream &operator<<(std::ostream &os, const TextureMap &point);
};
