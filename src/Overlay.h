#pragma once

#include "Common.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include<functional> 

struct UIContext {
    DrawingWindow& window;
    glm::vec2 currentPosition;
    std::function <void(glm::vec2 size)> renderHook;
    TTF_Font* font;

    UIContext(DrawingWindow& window) :
        window(window), currentPosition(glm::vec2()), font(nullptr) {
        renderHook = [](glm::vec2 _) -> void {};
    };
};

namespace overlay {
    void column(UIContext& ctx, std::function <void()> content) {
        auto prev = ctx.renderHook;
        glm::vec2 contentSize;
        ctx.renderHook = [&ctx, &contentSize](glm::vec2 size) {
            ctx.currentPosition.y += size.y;
            contentSize.y += size.y;
            contentSize.x = max(contentSize.x, size.x);
        };
        ctx.currentPosition = ctx.currentPosition - contentSize;
        content();
        ctx.renderHook = prev;
        prev(contentSize);
    }

    void setFont(UIContext& ctx, char* fontName, int size) {
        TTF_Font* font = TTF_OpenFont(fontName, size);
        if (!font) std::cout << "Couldn't find/init open ttf font." << std::endl;
        if (ctx.font) TTF_CloseFont(ctx.font);
        ctx.font = font;
    }

    void text(UIContext& ctx, std::string text, glm::vec3 color) {
        auto surface = TTF_RenderText_Solid(ctx.font, text.c_str(), {(uint8_t)color.r, (uint8_t)color.g, (uint8_t)color.b, 255});
        if (surface == nullptr) {
            std::cout << TTF_GetError() << std::endl;
            return;
        }
        auto texture = SDL_CreateTextureFromSurface(ctx.window.renderer, surface);
        SDL_Rect rect{ (int)ctx.currentPosition.x, (int)ctx.currentPosition.y, surface->w, surface->h };
        SDL_RenderCopy(ctx.window.renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        ctx.renderHook(glm::vec2(surface->w, surface->h));
    }
};

class Overlay {
    UIContext* context;

public:
    Overlay(DrawingWindow& window) {
        context = new UIContext(window);
        TTF_Init();
    }

    UIContext& resetContext() {
        context->currentPosition = glm::vec2(0);
        return *context;
    }

    ~Overlay() {
        TTF_Quit();
        if (context->font != nullptr) TTF_CloseFont(context->font);
        delete context;
    }
};
