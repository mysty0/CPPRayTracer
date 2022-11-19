#include "Common.h"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>

struct Context {
    DrawingWindow& window;
    glm::vec2 currentPosition;
    void (*renderHook)(glm::vec2 size);
    TTF_Font* font;
}

namespace overlay {
    void column(Context& ctx, void (*content)()) {
        auto prev = ctx.renderHook;
        glm::vec2 contentSize;
        ctx.renderHook = [](glm::vec2 size) { 
            ctx.currentPosition.y += size.y;
            contentSize.y += size.y;
            contentSize.x = max(contentSize.x, size.x);
        };
        ctx.currentPosition.y -= contentSize.y;
        content();
        ctx.renderHook = prev;
        prev(conetntSize);
    }

    void text(Context& ctx, std::string text, glm::vec3 color) {
        auto surface = TTF_RenderText_Solid(font, text, { color.r, color.g, color.b, 255 });
        if(surface == nullptr) {
            std::cout << TTF_GetError() << std::endl;
            return;
        }
        auto texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect{(int)ctx.currentPosition.x, (int)ctx.currentPosition.y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        ctx.renderHook(glm::vec2(surface->w, surface->h));
    }
}

class Overlay {
    
}