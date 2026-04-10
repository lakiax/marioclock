#pragma once
#include <stdint.h>
#include <stdbool.h>

// 物理屏幕（显存）的真实分辨率
#define SCREEN_WIDTH 284
#define SCREEN_HEIGHT 76

// PC 仿真时的画面放大倍数 (1 为原始大小不放大，想要放大改为 2 或 3)
#define DISPLAY_SCALE 1 

// 虚拟按键掩码
#define BTN_LEFT  (1 << 0)
#define BTN_RIGHT (1 << 1)
#define BTN_JUMP  (1 << 2)

void HAL_Init(void);
void HAL_UpdateScreen(const uint16_t* framebuffer);
uint8_t HAL_GetInput(void);
void HAL_Delay(uint32_t ms);
uint32_t HAL_GetTicks(void);