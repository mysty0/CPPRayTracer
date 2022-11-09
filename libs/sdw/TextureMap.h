#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Utils.h"

class TextureMap {
public:
	size_t width = 0;
	size_t height = 0;
	std::vector<uint32_t> pixels;

	TextureMap();
	TextureMap(const std::string &filename);
	uint32_t point(size_t x, size_t y) const;
	friend std::ostream &operator<<(std::ostream &os, const TextureMap &point);
};
