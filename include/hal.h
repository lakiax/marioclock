#pragma once
#include <stdint.h>
#include <stdbool.h>

// ============================================
// C/C++ 混合编译支持
// ============================================
#ifdef __cplusplus
extern "C" {
#endif

// ============================================
// 屏幕分辨率定义
// 物理屏幕是76x284的竖屏，但游戏使用横屏坐标系284x76
// 通过ST7789的MV位旋转90度实现横屏显示
// ============================================
#define SCREEN_WIDTH 284
#define SCREEN_HEIGHT 76

// PC 仿真时的画面放大倍数 (1 为原始大小，2 或 3 为放大)
#define DISPLAY_SCALE 1 

// 虚拟按键掩码
#define BTN_LEFT  (1 << 0)
#define BTN_RIGHT (1 << 1)
#define BTN_JUMP  (1 << 2)

// ============================================
// HAL 公共接口
// ============================================

// 初始化硬件抽象层（初始化屏幕、按键等）
void HAL_Init(void);

// 清屏（用指定颜色填充整个屏幕缓冲区）
void HAL_ClearScreen(uint16_t color);

// 更新屏幕显示（将帧缓冲区内容发送到屏幕）
void HAL_UpdateScreen(const uint16_t* framebuffer);

// 获取按键输入状态
uint8_t HAL_GetInput(void);

// 延时函数（毫秒）
void HAL_Delay(uint32_t ms);

// 获取系统运行时间（毫秒）
uint32_t HAL_GetTicks(void);

#ifdef __cplusplus
}
#endif
