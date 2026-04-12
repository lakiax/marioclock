// ============ 文件：src/hal_esp32s3.cpp ============
// ESP32-S3 硬件抽象层实现
// 用于驱动 76x284 ST7789P3 TFT 屏幕

#ifdef ESP32S3_TARGET

#include "hal.h"
#include <Arduino.h>
#include <SPI.h>

// ================= 引脚定义 =================
#define TFT_MOSI 5
#define TFT_SCLK 6
#define TFT_CS   7
#define TFT_DC   4
#define TFT_RST  15

// ================= ST7789P3 屏幕参数 =================
#define TFT_NATIVE_WIDTH  76
#define TFT_NATIVE_HEIGHT 284

// ST7789P3 切割屏幕的原厂偏移量
#define TFT_X_OFFSET 82
#define TFT_Y_OFFSET 18

// ================= 颜色定义 (RGB565) =================
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000   
#define COLOR_BLUE    0x001F  
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0

// 按键引脚定义
#define BTN_LEFT_PIN  10
#define BTN_RIGHT_PIN 11
#define BTN_JUMP_PIN  12

// ================= 硬件控制宏 =================
#define TFT_CS_0()  digitalWrite(TFT_CS, LOW)
#define TFT_CS_1()  digitalWrite(TFT_CS, HIGH)
#define TFT_DC_0()  digitalWrite(TFT_DC, LOW)
#define TFT_DC_1()  digitalWrite(TFT_DC, HIGH)
#define TFT_RST_0() digitalWrite(TFT_RST, LOW)
#define TFT_RST_1() digitalWrite(TFT_RST, HIGH)

// ================= 内部函数声明 =================
static void TFT_SEND_CMD(uint8_t cmd);
static void TFT_SEND_DATA(uint8_t data);
static void TFT_SET_WINDOW(uint16_t X, uint16_t Y, uint16_t X_END, uint16_t Y_END);
static void TFT_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
static void TFT_Init(void);

// ================= ST7789 底层通信 =================
static void TFT_SEND_CMD(uint8_t cmd) {
    TFT_CS_0();
    TFT_DC_0();
    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(cmd);
    SPI.endTransaction();
    TFT_CS_1();
}

static void TFT_SEND_DATA(uint8_t data) {
    TFT_CS_0();
    TFT_DC_1();
    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(data);
    SPI.endTransaction();
    TFT_CS_1();
}

static void TFT_SET_WINDOW(uint16_t X, uint16_t Y, uint16_t X_END, uint16_t Y_END) {
    uint16_t start_x = X + TFT_X_OFFSET;
    uint16_t end_x   = X_END + TFT_X_OFFSET;
    uint16_t start_y = Y + TFT_Y_OFFSET;
    uint16_t end_y   = Y_END + TFT_Y_OFFSET;

    TFT_SEND_CMD(0x2A);
    TFT_SEND_DATA(start_x >> 8);
    TFT_SEND_DATA(start_x & 0xFF);
    TFT_SEND_DATA(end_x >> 8);
    TFT_SEND_DATA(end_x & 0xFF);

    TFT_SEND_CMD(0x2B);
    TFT_SEND_DATA(start_y >> 8);
    TFT_SEND_DATA(start_y & 0xFF);
    TFT_SEND_DATA(end_y >> 8);
    TFT_SEND_DATA(end_y & 0xFF);

    TFT_SEND_CMD(0x2C);
}

static void TFT_Fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    TFT_SET_WINDOW(x, y, x + width - 1, y + height - 1);
    
    TFT_CS_0();
    TFT_DC_1();
    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));

    uint16_t color_swap = (color >> 8) | (color << 8);
    uint32_t pixels = width * height;
    
    uint16_t buffer[128];
    for(int i = 0; i < 128; i++) {
        buffer[i] = color_swap;
    }
    
    while(pixels > 0) {
        uint32_t chunk = (pixels > 128) ? 128 : pixels;
        SPI.writeBytes((uint8_t*)buffer, chunk * 2);
        pixels -= chunk;
    }
    
    SPI.endTransaction();
    TFT_CS_1();
}

static void TFT_Init(void) {
    TFT_RST_0();
    delay(100);
    TFT_RST_1();
    delay(100);

    TFT_SEND_CMD(0x11);
    delay(120);

    // 不旋转，使用默认竖屏方向
    TFT_SEND_CMD(0x36);
    TFT_SEND_DATA(0x00);  // 竖屏，不旋转

    TFT_SEND_CMD(0x3A);
    TFT_SEND_DATA(0x05);

    TFT_SEND_CMD(0xB2);
    TFT_SEND_DATA(0x05);
    TFT_SEND_DATA(0x05);
    TFT_SEND_DATA(0x00);
    TFT_SEND_DATA(0x33);
    TFT_SEND_DATA(0x33);

    TFT_SEND_CMD(0xB7);
    TFT_SEND_DATA(0x35);

    TFT_SEND_CMD(0xBB);
    TFT_SEND_DATA(0x21);

    TFT_SEND_CMD(0xC0);
    TFT_SEND_DATA(0x2C);

    TFT_SEND_CMD(0xC2);
    TFT_SEND_DATA(0x01);

    TFT_SEND_CMD(0xC3);
    TFT_SEND_DATA(0x0B);

    TFT_SEND_CMD(0xC4);
    TFT_SEND_DATA(0x20);

    TFT_SEND_CMD(0xC6);
    TFT_SEND_DATA(0x0F);

    TFT_SEND_CMD(0xD0);
    TFT_SEND_DATA(0xA7);
    TFT_SEND_DATA(0xA1);

    TFT_SEND_CMD(0xD0);
    TFT_SEND_DATA(0xA4);
    TFT_SEND_DATA(0xA1);

    TFT_SEND_CMD(0xD6);
    TFT_SEND_DATA(0xA1);

    TFT_SEND_CMD(0xE0);
    TFT_SEND_DATA(0xD0);
    TFT_SEND_DATA(0x04);
    TFT_SEND_DATA(0x08);
    TFT_SEND_DATA(0x0A);
    TFT_SEND_DATA(0x09);
    TFT_SEND_DATA(0x05);
    TFT_SEND_DATA(0x2D);
    TFT_SEND_DATA(0x43);
    TFT_SEND_DATA(0x49);
    TFT_SEND_DATA(0x09);
    TFT_SEND_DATA(0x16);
    TFT_SEND_DATA(0x15);
    TFT_SEND_DATA(0x26);
    TFT_SEND_DATA(0x2B);

    TFT_SEND_CMD(0xE1);
    TFT_SEND_DATA(0xD0);
    TFT_SEND_DATA(0x03);
    TFT_SEND_DATA(0x09);
    TFT_SEND_DATA(0x0A);
    TFT_SEND_DATA(0x0A);
    TFT_SEND_DATA(0x06);
    TFT_SEND_DATA(0x2E);
    TFT_SEND_DATA(0x44);
    TFT_SEND_DATA(0x40);
    TFT_SEND_DATA(0x3A);
    TFT_SEND_DATA(0x15);
    TFT_SEND_DATA(0x15);
    TFT_SEND_DATA(0x26);
    TFT_SEND_DATA(0x2A);

    TFT_SEND_CMD(0x20);
    delay(10);
    TFT_SEND_CMD(0x29);
    delay(10);
}

static void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    TFT_Fill(x, y, w, 1, color);
    TFT_Fill(x, y + h - 1, w, 1, color);
    TFT_Fill(x, y, 1, h, color);
    TFT_Fill(x + w - 1, y, 1, h, color);
}

#ifdef TFT_TEST_PATTERN
static void TFT_TestPattern(void) {
    TFT_Fill(0, 0, TFT_NATIVE_WIDTH, TFT_NATIVE_HEIGHT, COLOR_BLACK);
    TFT_DrawRect(0, 0, TFT_NATIVE_WIDTH, TFT_NATIVE_HEIGHT, COLOR_RED);
    TFT_Fill(15, 60,  46, 30, COLOR_RED);
    TFT_Fill(15, 120, 46, 30, COLOR_GREEN);
    TFT_Fill(15, 180, 46, 30, COLOR_BLUE);
    TFT_Fill(33, 240, 10, 10, COLOR_WHITE);
}
#endif

extern "C" {

void HAL_Init(void) {
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
    pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
    pinMode(BTN_JUMP_PIN, INPUT_PULLUP);

    TFT_CS_1();
    TFT_DC_1();
    TFT_RST_1();

    SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);

    TFT_Init();
    HAL_ClearScreen(COLOR_BLACK);
    
    #ifdef TFT_TEST_PATTERN
    TFT_TestPattern();
    delay(3000);
    #endif
}

void HAL_ClearScreen(uint16_t color) {
    TFT_Fill(0, 0, TFT_NATIVE_WIDTH, TFT_NATIVE_HEIGHT, color);
}

// 参考TFT_76_284例程的HAL_UpdateScreen实现
// 不旋转，直接填充屏幕（竖屏76宽 x 284高）
// 游戏画面会被裁剪/拉伸以适应屏幕
void HAL_UpdateScreen(const uint16_t* framebuffer) {
    // 设置全屏窗口（竖屏76x284）
    TFT_SET_WINDOW(0, 0, TFT_NATIVE_WIDTH - 1, TFT_NATIVE_HEIGHT - 1);
    
    TFT_CS_0();
    TFT_DC_1();
    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
    
    // 旋转映射：将横屏游戏画面（284x76）顺时针旋转90度以适应竖屏屏幕（76x284）
    for (int y = 0; y < TFT_NATIVE_HEIGHT; y++) {
        for (int x = 0; x < TFT_NATIVE_WIDTH; x++) {
            // 游戏画面的列对应屏幕的行
            int src_col = y;  // 游戏列 = 屏幕行
            int src_row = SCREEN_HEIGHT - 1 - x;  // 游戏行 = 屏幕高度 - 1 - 屏幕列
            
            if (src_row >= 0 && src_row < SCREEN_HEIGHT && src_col >= 0 && src_col < SCREEN_WIDTH) {
                uint16_t color = framebuffer[src_row * SCREEN_WIDTH + src_col];
                SPI.write16(color);
            } else {
                // 如果映射超出范围，发送黑色
                SPI.write16(0x0000);
            }
        }
    }
    
    SPI.endTransaction();
    TFT_CS_1();
}

uint8_t HAL_GetInput(void) {
    uint8_t btn_state = 0;
    if (digitalRead(BTN_LEFT_PIN) == LOW)  btn_state |= BTN_LEFT;
    if (digitalRead(BTN_RIGHT_PIN) == LOW) btn_state |= BTN_RIGHT;
    if (digitalRead(BTN_JUMP_PIN) == LOW)  btn_state |= BTN_JUMP;
    return btn_state;
}

void HAL_Delay(uint32_t ms) {
    delay(ms);
}

uint32_t HAL_GetTicks(void) {
    return millis();
}

}

#endif // ESP32S3_TARGET
