#include "particle.h"
#include "engine.h"
#include "hal.h"
#include "mario_sprites.h"

static Particle particles[MAX_PARTICLES] = {0};

// 3x5 点阵像素字体映射表 (0-9)
static const uint8_t font_3x5[10][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
    {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}
};

#define BRICK_W 10                // 与 digit.c 保持一致
#define DEBRIS_SIZE 5             // 碎屑尺寸：数字碎裂后每块碎片在屏幕上实际显示大小，增大可让碎片更大更清晰
#define SHATTER_GRAVITY 0.4f      // 碎屑重力加速度，增大后碎片下坠更快，减小可让飞散更缓慢

#define SHATTER_VX_UL -1.0f       // 左上碎屑初速度 X
#define SHATTER_VY_UL -3.0f       // 左上碎屑初速度 Y
#define SHATTER_VX_UR 1.0f        // 右上碎屑初速度 X
#define SHATTER_VY_UR -3.0f       // 右上碎屑初速度 Y
#define SHATTER_VX_LL -1.2f       // 左下碎屑初速度 X
#define SHATTER_VY_LL -1.8f       // 左下碎屑初速度 Y
#define SHATTER_VX_LR 1.2f        // 右下碎屑初速度 X
#define SHATTER_VY_LR -1.8f       // 右下碎屑初速度 Y
#define SHATTER_DEBRIS_SRC_HALF 4 // 8x8碎屑区域的一半，用于计算图集源偏移

void Particle_Init(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

void Particle_SpawnBrickDebris(int x, int y) {
    // 碎屑在大图集中的位置 (第3列，src_x = 32)
    int atlas_debris_x = 32;
    int atlas_debris_y = 0;

    // 4 个碎屑的爆炸初速度 (左上，右上，左下，右下)
    float v_config[4][2] = {{SHATTER_VX_UL, SHATTER_VY_UL}, {SHATTER_VX_UR, SHATTER_VY_UR}, {SHATTER_VX_LL, SHATTER_VY_LL}, {SHATTER_VX_LR, SHATTER_VY_LR}};
    // 位置偏移（基于碎屑尺寸）
    int pos_offset[4][2] = {{0, 0}, {DEBRIS_SIZE, 0}, {0, DEBRIS_SIZE}, {DEBRIS_SIZE, DEBRIS_SIZE}};
    // 图集源偏移（8x8碎屑区域的一半=4）
    int src_offset[4][2] = {{0, 0}, {SHATTER_DEBRIS_SRC_HALF, 0}, {0, SHATTER_DEBRIS_SRC_HALF}, {SHATTER_DEBRIS_SRC_HALF, SHATTER_DEBRIS_SRC_HALF}};

    for (int i = 0; i < 4; i++) {
        // 在池子里找一个空闲的粒子
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                particles[p].active = true;
                particles[p].x = x + pos_offset[i][0];
                particles[p].y = y + pos_offset[i][1];
                particles[p].vx = v_config[i][0];
                particles[p].vy = v_config[i][1];
                // 指向大图集里碎屑对应的 8x8 区域
                particles[p].src_x = atlas_debris_x + src_offset[i][0];
                particles[p].src_y = atlas_debris_y + src_offset[i][1];
                break;
            }
        }
    }
}

void Particle_ShatterDigit(int sx, int sy, int digit) {
    int bw = BRICK_W;
    for (int row = 0; row < 5; row++) {
        uint8_t line = font_3x5[digit][row];
        for (int col = 0; col < 3; col++) {
            if (line & (1 << (2 - col))) {
                Particle_SpawnBrickDebris(sx + col * bw, sy + row * bw);
            }
        }
    }
}

void Particle_Update(void) {
    for (int p = 0; p < MAX_PARTICLES; p++) {
        if (particles[p].active) {
            // 物理更新
            particles[p].vy += SHATTER_GRAVITY; // 碎屑的重力比马里奥稍轻一点，飞得更好看
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;
            
            // 如果掉出屏幕，回收粒子
            if (particles[p].y > SCREEN_HEIGHT) {
                particles[p].active = false;
            }
        }
    }
}

void Particle_Render(const uint8_t* atlas) {
    for (int p = 0; p < MAX_PARTICLES; p++) {
        if (particles[p].active) {
            // 绘制碎屑（将8x8图集源缩放到DEBRIS_SIZE×DEBRIS_SIZE）
            Engine_DrawScaledAtlasSprite((int)particles[p].x, (int)particles[p].y, DEBRIS_SIZE, DEBRIS_SIZE,
                particles[p].src_x, particles[p].src_y, 8, 8, 80, atlas);
        }
    }
}
