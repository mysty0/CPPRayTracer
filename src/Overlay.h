#pragma once

#include "Common.h"
#include <SDL_ttf.h>
#include <SDL.h>
#include<functional> 

struct UIContext {
    DrawingWindow& window;
    glm::ivec2 currentPosition;
    std::function <void(glm::vec2 size)> renderHook;
    TTF_Font* font;

    UIContext(DrawingWindow& window) :
        window(window), currentPosition(glm::ivec2()), font(nullptr) {
        renderHook = [](glm::ivec2 _) -> void {};
    };
};

namespace overlay {
    void text(UIContext* ctx, std::string text, glm::vec3 color) {
        auto surface = TTF_RenderText_Solid(ctx->font, text.c_str(), { (uint8_t)color.r, (uint8_t)color.g, (uint8_t)color.b, 255 });
        if (surface == nullptr) {
            std::cout << TTF_GetError() << std::endl;
            return;
        }
        auto texture = SDL_CreateTextureFromSurface(ctx->window.renderer, surface);
        //std::cout << " " << surface->w << " " << surface->h << " " << text << std::endl;
        ctx->renderHook(glm::ivec2(surface->w, surface->h));
        SDL_Rect rect{ (int)ctx->currentPosition.x, (int)ctx->currentPosition.y, surface->w, surface->h };
        SDL_RenderCopy(ctx->window.renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        //std::cout << surface->h << std::endl;
        
    }

    void column(UIContext* ctx, std::function <void()> content) {
        auto prev = ctx->renderHook;
        glm::ivec2 contentSize;
        auto startPos = ctx->currentPosition;
        ctx->renderHook = [&](glm::ivec2 size) {
            ctx->currentPosition.y += size.y;
            contentSize.y += size.y;
            contentSize.x = max(contentSize.x, size.x);
        };
        content();
        ctx->currentPosition = startPos;
        //ctx.currentPosition.x += contentSize.x;
        //auto pos = ctx.currentPosition;
        //text(ctx, string_format("%d %d : %d %d", pos.x, pos.y, contentSize.x, contentSize.y), glm::vec3(255));
        //ctx.currentPosition.x -= contentSize.x;
        ctx->renderHook = prev;
        prev(contentSize);
    }

    void setFont(UIContext* ctx, char* fontName, int size) {
        TTF_Font* font = TTF_OpenFont(fontName, size);
        if (!font) std::cout << "Couldn't find/init open ttf font." << std::endl;
        if (ctx->font) TTF_CloseFont(ctx->font);
        ctx->font = font;
    }
};

class Overlay {
    UIContext* context;

public:
    Overlay(DrawingWindow& window) {
        std::cout << "init overlay" << std::endl;
        context = new UIContext(window);
        TTF_Init();
    }

    UIContext* resetContext() {
        context->currentPosition = glm::vec2(0);
        return context;
    }

    ~Overlay() {
        TTF_Quit();
        if (context->font != nullptr) TTF_CloseFont(context->font);
        delete context;
    }
};
