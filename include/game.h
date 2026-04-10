#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/**
 * 游戏初始化
 */
void Game_Init(void);

/**
 * 记录每帧的开始时间
 */
void Game_StartFrame(void);

/**
 * 处理输入任务
 */
void Game_TaskInput(void);

/**
 * 更新物理任务
 */
void Game_TaskPhysicsUpdate(void);

/**
 * 更新状态机任务（数字、粒子等）
 */
void Game_TaskUpdateStateMachine(void);

/**
 * 渲染任务
 */
void Game_TaskRender(void);

/**
 * 帧率控制任务
 */
void Game_TaskFrameRateControl(void);

#endif // GAME_H
