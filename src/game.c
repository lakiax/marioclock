#include <math.h>
#include <time.h>
#include "game.h"
#include "hal.h"
#include "engine.h"
#include "mario.h"
#include "particle.h"
#include "digit.h"
#include "mario_sprites.h"

// 全局游戏状态
static struct {
    Character mario;
    uint32_t current_ticks;
    uint32_t frame_start_ticks;
    time_t current_time;
    struct tm* tm_p;
    // FPS 计算相关
    uint32_t frame_count;
    uint32_t last_fps_time;
    uint32_t current_fps;
} game_state = {0};

void Game_Init(void) {
    HAL_Init();
    
    // 初始化各模块
    Mario_Init(&game_state.mario, 20, 60);
    Particle_Init();
    Digit_Init();
    
    // 获取初始时间，初始化 4 个数字模块
    time(&game_state.current_time);
    game_state.tm_p = localtime(&game_state.current_time);
    Digit_InitFromTime(game_state.tm_p, 5);
}

void Game_TaskInput(void) {
    uint8_t btn = HAL_GetInput();
    Mario_UpdateInput(&game_state.mario, btn);
}

void Game_TaskPhysicsUpdate(void) {
    // 先记住当前位置，更新物理后再检查碰撞
    float previous_x = game_state.mario.x;
    float previous_y = game_state.mario.y;

    Mario_UpdatePhysics(&game_state.mario);

    float adjusted_x, adjusted_y;
    int collision_type = Digit_CheckCollision(previous_x, previous_y, game_state.mario.vx, game_state.mario.vy,
                                             &adjusted_x, &adjusted_y);

    if (collision_type > 0) {
        game_state.mario.x = adjusted_x;
        game_state.mario.y = adjusted_y;

        if (collision_type == 1) {
            game_state.mario.vx = 0;
        } else if (collision_type == 2) {
            // 落到砖块上
            game_state.mario.vy = 0;
            game_state.mario.is_jumping = false;
        } else if (collision_type == 3) {
            // 头顶撞到砖块
            game_state.mario.vy = 0;
            // 保持 is_jumping = true，因为还在空中，重力会让他落下
        }
    }

    // 累积移动距离（用于动画）
    if (fabs(game_state.mario.vx) > 0.1f && !game_state.mario.is_jumping) {
        game_state.mario.distance_moved += fabs(game_state.mario.vx);
    }

    // 更新动画（基于最终状态）
    Mario_UpdateAnimation(&game_state.mario);
}

void Game_TaskUpdateStateMachine(void) {
    // 获取最新时间
    time(&game_state.current_time);
    game_state.tm_p = localtime(&game_state.current_time);
    
    // 更新数字状态机
    Digit_Update(game_state.current_ticks, game_state.tm_p);
    
    // 更新粒子物理
    Particle_Update();
}

void Game_TaskRender(void) {
    // 清空屏幕缓冲区
    Engine_ClearBuffer(0x5D1C);
    
    // 分层渲染
    // 1. 冒号层
    Digit_Render(game_state.current_ticks, game_state.tm_p, sprite_7_8_map, sprite_7_6_map);
    
    // 2. 粒子层（碎屑）
    Particle_Render(sprite_7_8_map);
    
    // 3. 前景层：马里奥
    Mario_Render(&game_state.mario);
    
    // 4. 左上角显示实时FPS (小字体)
    int fps = game_state.current_fps;
    if (fps > 999) fps = 999; // 限制为三位数
    int hundreds = fps / 100;
    int tens = (fps % 100) / 10;
    int ones = fps % 10;
    int x = 2;
    // 绘制百位数
    if (hundreds > 0) {
        Engine_DrawNumber(x, 2, hundreds, 0xFFFF);
        x += 4; // 数字宽度3 + 间距1
    }
    // 绘制十位数（如果百位数存在或十位数>0）
    if (hundreds > 0 || tens > 0) {
        Engine_DrawNumber(x, 2, tens, 0xFFFF);
        x += 4;
    }
    // 绘制个位数
    Engine_DrawNumber(x, 2, ones, 0xFFFF);
    
    // 更新屏幕显示
    HAL_UpdateScreen(screen_buffer);
}

void Game_TaskFrameRateControl(void) {
    // 锁帧 50 FPS (20 ms/frame)
    uint32_t cost = HAL_GetTicks() - game_state.frame_start_ticks;
    if (cost < 20) {
        HAL_Delay(20 - cost);
    }
}

// 每帧循环开始的记录，用于各任务
void Game_StartFrame(void) {
    game_state.current_ticks = HAL_GetTicks();
    game_state.frame_start_ticks = game_state.current_ticks;

    // FPS 计算
    game_state.frame_count++;
    uint32_t now = game_state.current_ticks;
    if (now - game_state.last_fps_time >= 1000) {
        game_state.current_fps = game_state.frame_count;
        game_state.frame_count = 0;
        game_state.last_fps_time = now;
    }
}
