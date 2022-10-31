#include "Colour.h"
#include <utility>

Colour::Colour(): red(0), green(0), blue(0) {}
Colour::Colour(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << "["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}
