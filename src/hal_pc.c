#ifdef PC_SIMULATOR
#include "hal.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

void HAL_Init(void) {
    SDL_Init(SDL_INIT_VIDEO);
    
    window = SDL_CreateWindow("Mario Physics Simulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH * DISPLAY_SCALE, SCREEN_HEIGHT * DISPLAY_SCALE, SDL_WINDOW_SHOWN);
        
    // 移除垂直同步 (VSYNC) 以实现最快渲染速度
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); 
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, 
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void HAL_UpdateScreen(const uint16_t* framebuffer) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) exit(0); }
    
    // 内部 framebuffer 永远是 284x76，SDL 会自动帮我们拉伸到窗口大小
    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * 2);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

uint8_t HAL_GetInput(void) {
    uint8_t btn_state = 0;
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT])  btn_state |= BTN_LEFT;
    if (state[SDL_SCANCODE_RIGHT]) btn_state |= BTN_RIGHT;
    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_SPACE]) btn_state |= BTN_JUMP;
    return btn_state;
}

void HAL_ClearScreen(uint16_t color) {
    // PC端没有单独的缓冲区清屏操作，由Engine_ClearBuffer处理
    // 此函数在ESP32端用于直接操作显存
    (void)color;
}

void HAL_Delay(uint32_t ms) { SDL_Delay(ms); }
uint32_t HAL_GetTicks(void) { return SDL_GetTicks(); }
#endif
