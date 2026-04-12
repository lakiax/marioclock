#ifndef MARIO_H
#define MARIO_H

#include <stdint.h>
#include <stdbool.h>
#include "engine.h"  // 包含 Animation 和 Character 类型定义

// 物理常量
#define GRAVITY 0.55f
#define JUMP_FORCE -6.2f
#define MOVE_ACCEL 0.16f
#define MAX_SPEED 2.5f
#define GROUND_Y 60
#define PIXELS_PER_FRAME 6.0f

/**
 * 初始化马里奥角色
 * @param mario 角色指针
 * @param x, y 初始位置
 */
void Mario_Init(Character* mario, float x, float y);

/**
 * 处理输入和物理更新
 * @param mario 角色指针
 * @param btn 按键输入
 */
void Mario_UpdateInput(Character* mario, uint8_t btn);

/**
 * 更新物理（重力、碰撞）
 * @param mario 角色指针
 */
void Mario_UpdatePhysics(Character* mario);

/**
 * 更新动画状态
 * @param mario 角色指针
 */
void Mario_UpdateAnimation(Character* mario);

/**
 * 渲染马里奥
 * @param mario 角色指针
 */
void Mario_Render(const Character* mario);

#endif // MARIO_H
