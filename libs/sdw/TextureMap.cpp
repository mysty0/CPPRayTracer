#include "TextureMap.h"

TextureMap::TextureMap() = default;
TextureMap::TextureMap(const std::string &filename) {
	std::ifstream inputStream(filename, std::ifstream::binary);
	std::string nextLine;
	// Get the "P6" magic number
	std::getline(inputStream, nextLine);
	// Read the width and height line
	std::getline(inputStream, nextLine);
	// Skip over any comment lines !
	while (nextLine.size() == 0 || nextLine.at(0) == '#') std::getline(inputStream, nextLine);
	auto widthAndHeight = split(nextLine, ' ');
	if (widthAndHeight.size() != 2)
		throw std::invalid_argument("Failed to parse width and height line, line was `" + nextLine + "`");

	width = std::stoi(widthAndHeight[0]);
	height = std::stoi(widthAndHeight[1]);
	// Read the max value (which we assume is 255)
	std::getline(inputStream, nextLine);

	pixels.resize(width * height);
	for (size_t i = 0; i < width * height; i++) {
		int red = inputStream.get();
		int green = inputStream.get();
		int blue = inputStream.get();
		pixels[i] = ((255 << 24) + (red << 16) + (green << 8) + (blue));
	}
	inputStream.close();
}

std::ostream &operator<<(std::ostream &os, const TextureMap &map) {
	os << "(" << map.width << " x " << map.height << ")";
	return os;
}

uint32_t TextureMap::point(size_t x, size_t y) const {
	if(x >= width || y >= height) {
		//std::cout << x << "," << y << " out of texture bounds" << std::endl;
		return 0;
	}
	return this->pixels[x + y * this->width];
}

uint32_t TextureMap::point(glm::vec2 pos) const {
	if (pos.x >= 1 || pos.y >= 1) {
		//std::cout << x << "," << y << " out of texture bounds" << std::endl;
		return 0;
	}
	int ind = (pos.x) * this->width + ((pos.y) * this->height) * this->width;
	if (ind > this->pixels.size()) {
		return 0;
	}

	return this->pixels[ind];
}
