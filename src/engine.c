// ============ 文件：src/engine.c ============
#include "engine.h"
#include "hal.h"

uint16_t screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
#define COLOR_TRANSPARENT 0x949F // LVGL RGB565 小端的透明背景色

// 3x5 数字字体 (0-9)，每行3位，最高位在左边
static const uint8_t font_3x5[10][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
    {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}
};

void Engine_ClearBuffer(uint16_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) screen_buffer[i] = color;
}

// 核心功能：支持向左转向（镜像翻转）的绘图函数
void Engine_DrawSprite(int sx, int sy, int sw, int sh, const uint8_t* img_data, bool flip_h) {
    const uint16_t* pixels = (const uint16_t*)img_data;

    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            // 如果要求水平翻转，从右边倒着读像素
            int src_x = flip_h ? (sw - 1 - x) : x; 
            uint16_t pixel = pixels[y * sw + src_x];
            
            if (pixel != COLOR_TRANSPARENT) {
                int targetX = sx + x;
                int targetY = sy + y;
                if (targetX >= 0 && targetX < SCREEN_WIDTH && targetY >= 0 && targetY < SCREEN_HEIGHT) {
                    screen_buffer[targetY * SCREEN_WIDTH + targetX] = pixel;
                }
            }
        }
    }
}

void Engine_DrawAtlasSprite(int sx, int sy, int sw, int sh, int src_x, int src_y, int atlas_w, const uint8_t* img_data) {
    const uint16_t* pixels = (const uint16_t*)img_data;
    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            // 计算在大图集数组中的位置
            uint16_t pixel = pixels[(src_y + y) * atlas_w + (src_x + x)];
            if (pixel != COLOR_TRANSPARENT) {
                int targetX = sx + x;
                int targetY = sy + y;
                if (targetX >= 0 && targetX < SCREEN_WIDTH && targetY >= 0 && targetY < SCREEN_HEIGHT) {
                    screen_buffer[targetY * SCREEN_WIDTH + targetX] = pixel;
                }
            }
        }
    }
}

static uint16_t BlendRgb565(uint16_t src, uint16_t dst, uint8_t alpha_percent) {
    // alpha_percent 取值 0~100，0 完全透明，100 完全不透明
    uint8_t src_r = (src >> 11) & 0x1F;
    uint8_t src_g = (src >> 5) & 0x3F;
    uint8_t src_b = src & 0x1F;
    uint8_t dst_r = (dst >> 11) & 0x1F;
    uint8_t dst_g = (dst >> 5) & 0x3F;
    uint8_t dst_b = dst & 0x1F;

    uint8_t out_r = (src_r * alpha_percent + dst_r * (100 - alpha_percent) + 50) / 100;
    uint8_t out_g = (src_g * alpha_percent + dst_g * (100 - alpha_percent) + 50) / 100;
    uint8_t out_b = (src_b * alpha_percent + dst_b * (100 - alpha_percent) + 50) / 100;

    return (out_r << 11) | (out_g << 5) | out_b;
}

void Engine_DrawScaledAtlasSprite(int sx, int sy, int dw, int dh, int src_x, int src_y, int sw, int sh, int atlas_w, const uint8_t* img_data) {
    const uint16_t* pixels = (const uint16_t*)img_data;
    for (int y = 0; y < dh; y++) {
        // 最近邻缩放：将目标像素映射回源像素
        int src_row = y * sh / dh;
        for (int x = 0; x < dw; x++) {
            int src_col = x * sw / dw;
            uint16_t pixel = pixels[(src_y + src_row) * atlas_w + (src_x + src_col)];
            if (pixel != COLOR_TRANSPARENT) {
                int targetX = sx + x;
                int targetY = sy + y;
                if (targetX >= 0 && targetX < SCREEN_WIDTH && targetY >= 0 && targetY < SCREEN_HEIGHT) {
                    screen_buffer[targetY * SCREEN_WIDTH + targetX] = pixel;
                }
            }
        }
    }
}

void Engine_DrawScaledAtlasSpriteAlpha(int sx, int sy, int dw, int dh, int src_x, int src_y, int sw, int sh, int atlas_w, const uint8_t* img_data, uint8_t alpha) {
    const uint16_t* pixels = (const uint16_t*)img_data;
    if (alpha == 0) return;
    if (alpha > 100) alpha = 100;

    for (int y = 0; y < dh; y++) {
        int src_row = y * sh / dh;
        for (int x = 0; x < dw; x++) {
            int src_col = x * sw / dw;
            uint16_t pixel = pixels[(src_y + src_row) * atlas_w + (src_x + src_col)];
            if (pixel != COLOR_TRANSPARENT) {
                int targetX = sx + x;
                int targetY = sy + y;
                if (targetX >= 0 && targetX < SCREEN_WIDTH && targetY >= 0 && targetY < SCREEN_HEIGHT) {
                    uint16_t dst = screen_buffer[targetY * SCREEN_WIDTH + targetX];
                    screen_buffer[targetY * SCREEN_WIDTH + targetX] = BlendRgb565(pixel, dst, alpha);
                }
            }
        }
    }
}

// 绘制单个数字 (0-9)，使用 3x5 字体，每个像素放大为 1x1（小字体）
void Engine_DrawNumber(int x, int y, int number, uint16_t color) {
    if (number < 0 || number > 9) return;
    for (int row = 0; row < 5; row++) {
        uint8_t line = font_3x5[number][row];
        for (int col = 0; col < 3; col++) {
            if (line & (1 << (2 - col))) { // 最高位在左边
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    screen_buffer[py * SCREEN_WIDTH + px] = color;
                }
            }
        }
    }
}