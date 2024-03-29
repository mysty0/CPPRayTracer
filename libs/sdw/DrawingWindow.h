#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "SDL.h"

class DrawingWindow {

public:
	size_t width;
	size_t height;
	SDL_Renderer *renderer;

private:
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Texture* overlay;
	std::vector<uint32_t> pixelBuffer;
	std::vector<uint32_t> overlayPixelBuffer;

public:
	DrawingWindow();
	DrawingWindow(int w, int h, bool fullscreen);
	void renderFrame();
	void savePPM(const std::string &filename) const;
	void saveBMP(const std::string &filename) const;
	bool pollForInputEvents(SDL_Event &event);
	void setPixelColour(size_t x, size_t y, uint32_t colour);
	void setOverlayPixelColour(size_t x, size_t y, uint32_t colour);
	uint32_t getPixelColour(size_t x, size_t y);
	void clearPixels();
	void finishRender();
};

void printMessageAndQuit(const std::string &message, const char *error);
