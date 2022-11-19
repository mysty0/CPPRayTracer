#pragma once

#include <iostream>

struct Colour {
	uint8_t red{};
	uint8_t green{};
	uint8_t blue{};
	Colour();
	Colour(uint8_t r, uint8_t g, uint8_t b);
	friend Colour operator*(const Colour &a, float x);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
