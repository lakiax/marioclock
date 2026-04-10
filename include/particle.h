#ifndef PARTICLE_H
#define PARTICLE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PARTICLES 150

typedef struct {
    float x, y;
    float vx, vy;
    bool active;
    int src_x, src_y; // 截取大图集的坐标
} Particle;

/**
 * 初始化粒子系统
 */
void Particle_Init(void);

/**
 * 产生砖块碎屑 (16x16 砖块碎裂成 4 个 8x8 碎屑)
 * @param x, y 砖块左上角位置
 */
void Particle_SpawnBrickDebris(int x, int y);

/**
 * 产生数字碎裂效果
 * @param sx, sy 数字起始位置
 * @param digit 0-9 的数字
 */
void Particle_ShatterDigit(int sx, int sy, int digit);

/**
 * 更新所有粒子物理（重力、位置；清除超出屏幕的粒子）
 */
void Particle_Update(void);

/**
 * 渲染所有活跃粒子
 * @param atlas 图集数据指针
 */
void Particle_Render(const uint8_t* atlas);

#endif // PARTICLE_H
