#pragma once

#include <iostream>

struct Colour {
	uint8_t red{};
	uint8_t green{};
	uint8_t blue{};
	Colour();
	Colour(uint8_t r, uint8_t g, uint8_t b);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
