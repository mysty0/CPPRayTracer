#pragma once

//#include <SDL2/SDL_ttf.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "Colour.h"
#include "DrawingWindow.h"

#include <glm/glm.hpp>

namespace text {
    const char* FONT_NAME = "../../../Roboto-Regular.ttf";

    SDL_Rect renderText(SDL_Renderer* renderer, glm::vec2 position, const char* text, Colour color) {
        TTF_Init();
        TTF_Font* font = TTF_OpenFont(FONT_NAME, 12);
        if (!font) std::cout << "Couldn't find/init open ttf font." << std::endl;

        auto surface = TTF_RenderText_Solid(font, text, { color.red, color.green, color.blue, 255 });
        if(surface == nullptr) {
            std::cout << TTF_GetError() << std::endl;
            return {};
        }
        auto texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect{(int)position.x, (int)position.y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        TTF_Quit();
        return rect;
    }
}