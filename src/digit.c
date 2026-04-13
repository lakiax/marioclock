
#include <stdint.h>
#include "digit.h"

#define COLON_COIN_UP_TIME    80      // 上升时长(ms)
#define COLON_COIN_DOWN_TIME  80      // 下降回位时长(ms)
#define COLON_COIN_JUMP_HEIGHT (BRICK_W / 2) // 跳跃高度

static TimeDigit digits[4];
static Brick bricks[4][DIGIT_BRICK_COUNT];
static ColonCoin colon_coins[COLON_COIN_COUNT];

// 更新金币砖块动画状态
static void Digit_UpdateColonCoinAnimation(uint32_t current_ticks) {
    for (int i = 0; i < COLON_COIN_COUNT; i++) {
        ColonCoin* coin = &colon_coins[i];
        if (coin->state == COLON_COIN_STATE_UP) {
            uint32_t elapsed = current_ticks - coin->timer;
            if (elapsed < COLON_COIN_UP_TIME) {
                // 匀速上移到最高点
                coin->anim_offset_y = -(int)((COLON_COIN_JUMP_HEIGHT * elapsed) / COLON_COIN_UP_TIME);
            } else {
                // 上升结束，进入下降
                coin->state = COLON_COIN_STATE_DOWN;
                coin->timer = current_ticks;
                coin->anim_offset_y = -COLON_COIN_JUMP_HEIGHT;
            }
        } else if (coin->state == COLON_COIN_STATE_DOWN) {
            uint32_t elapsed = current_ticks - coin->timer;
            if (elapsed < COLON_COIN_DOWN_TIME) {
                // 匀速下移回原位
                coin->anim_offset_y = -COLON_COIN_JUMP_HEIGHT + (int)((COLON_COIN_JUMP_HEIGHT * elapsed) / COLON_COIN_DOWN_TIME);
            } else {
                // 回到原位
                coin->state = COLON_COIN_STATE_NORMAL;
                coin->anim_offset_y = 0;
            }
        } else {
            coin->anim_offset_y = 0;
        }

        if (coin->locked && current_ticks >= coin->locked_until) {
            coin->locked = 0;
        }
    }
}
#include <math.h>
#include "particle.h"
#include "engine.h"
#include "hal.h"

// 3x5 点阵像素字体映射表 (0-9)
static const uint8_t font_3x5[10][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
    {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}
};

#define BRICK_ORDER(row, col) (((DIGIT_BRICK_ROWS - 1 - (row)) * DIGIT_BRICK_COLS) + (DIGIT_BRICK_COLS - 1 - (col)))



static uint32_t XorShift32(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static void ShuffleBrickOrder(uint8_t* order, int count, uint32_t seed) {
    for (int i = 0; i < count; i++) {
        order[i] = (uint8_t)i;
    }
    for (int i = count - 1; i > 0; i--) {
        seed = XorShift32(seed);
        int j = seed % (i + 1);
        uint8_t tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }
}

static void ScheduleRandomShatter(int digit_idx, uint32_t current_ticks, int digit_value) {
    uint8_t order[DIGIT_BRICK_COUNT];
    ShuffleBrickOrder(order, DIGIT_BRICK_COUNT, current_ticks ^ 0xA5A5A5A5u);
    int step = 0;

    for (int i = 0; i < DIGIT_BRICK_COUNT; i++) {
        int idx = order[i];
        int row = idx / DIGIT_BRICK_COLS;
        int col = idx % DIGIT_BRICK_COLS;
        if (!(font_3x5[digit_value][row] & (1 << (2 - col)))) {
            continue;
        }
        bricks[digit_idx][idx].state = 3; // 待碎裂
        bricks[digit_idx][idx].timer = current_ticks + step * DIGIT_SHATTER_STAGGER_MS;
        step++;
    }
}

void Digit_Init(void) {
    for (int i = 0; i < 4; i++) {
        digits[i].val = 0;
        digits[i].next_val = 0;
        digits[i].state = 0;
        digits[i].timer = 0;
        digits[i].sx = 0;
        digits[i].sy = 0;
        // 初始化砖块
        for (int b = 0; b < DIGIT_BRICK_COUNT; b++) {
            bricks[i][b].state = 0;
            bricks[i][b].timer = 0;
        }
    }

    // 初始化冒号金币砖块
    for (int i = 0; i < COLON_COIN_COUNT; i++) {
        colon_coins[i].x = COLON_X;
        colon_coins[i].y = 0; // y 由 Digit_InitFromTime 设置
        colon_coins[i].state = COLON_COIN_STATE_NORMAL;
        colon_coins[i].timer = 0;
        colon_coins[i].anim_offset_y = 0;
        colon_coins[i].locked_until = 0;
        colon_coins[i].locked = 0;
    }
}

void Digit_InitFromTime(struct tm* tm_p, int offset_y) {
    // 缩小后重新计算水平位置，居中显示
    // 每个数字宽 3*BRICK_W=30，冒号宽 BRICK_W=10
    // 布局: D0(30) gap(4) D1(30) gap(10) colon(10) gap(10) D2(30) gap(4) D3(30) = 158
    // 起始X = (284 - 158) / 2 = 63
    digits[0] = (TimeDigit){tm_p->tm_hour / 10, tm_p->tm_hour / 10, 0, 0, 63, offset_y};
    digits[1] = (TimeDigit){tm_p->tm_hour % 10, tm_p->tm_hour % 10, 0, 0, 97, offset_y};
    digits[2] = (TimeDigit){tm_p->tm_min / 10,  tm_p->tm_min / 10, 0, 0, 157, offset_y};
    digits[3] = (TimeDigit){tm_p->tm_min % 10,  tm_p->tm_min % 10, 0, 0, 191, offset_y};
    
    // 初始化砖块位置
    for (int d = 0; d < 4; d++) {
        int idx = 0;
        for (int row = 0; row < DIGIT_BRICK_ROWS; row++) {
            for (int col = 0; col < DIGIT_BRICK_COLS; col++) {
                bricks[d][idx].x = digits[d].sx + col * BRICK_W;
                bricks[d][idx].y = digits[d].sy + row * BRICK_W;
                bricks[d][idx].state = 0;
                bricks[d][idx].timer = 0;
                idx++;
            }
        }
    }

    // 设置冒号金币砖块的 y 坐标
    colon_coins[0].y = offset_y + BRICK_W * 1;
    colon_coins[1].y = offset_y + BRICK_W * 3;

    colon_coins[0].anim_offset_y = 0;
    colon_coins[1].anim_offset_y = 0;
    colon_coins[0].state = COLON_COIN_STATE_NORMAL;
    colon_coins[1].state = COLON_COIN_STATE_NORMAL;
    colon_coins[0].timer = 0;
    colon_coins[1].timer = 0;
    colon_coins[0].locked_until = 0;
    colon_coins[1].locked_until = 0;
    colon_coins[0].locked = 0;
    colon_coins[1].locked = 0;
}

// 渲染 3x5 数字的砖块，支持从下到上、从右到左逐个砖块弹出并逐渐从透明到不透明
static void DrawBrickDigit3x5(int digit_idx, int sx, int sy, int digit, const uint8_t* atlas, uint32_t current_ticks, uint32_t fade_start_time) {
    int bw = BRICK_W;
    int idx = 0;
    for (int row = 0; row < DIGIT_BRICK_ROWS; row++) {
        uint8_t line = font_3x5[digit][row];
        for (int col = 0; col < DIGIT_BRICK_COLS; col++) {
            if (!(line & (1 << (2 - col)))) {
                idx++;
                continue;
            }

            Brick* brick = &bricks[digit_idx][idx];
            if (brick->state == 1) {
                idx++;
                continue;
            }

            if (fade_start_time == UINT32_MAX) {
                if (brick->state != 1) {
                    Engine_DrawScaledAtlasSprite(sx + col * bw, sy + row * bw, bw, bw, 0, 0, 16, 16, 80, atlas);
                }
            } else {
                int order = BRICK_ORDER(row, col);
                uint32_t brick_start = fade_start_time + order * DIGIT_BRICK_STAGGER_MS;
                if (current_ticks < brick_start) {
                    idx++;
                    continue;
                }

                uint32_t elapsed = current_ticks - brick_start;
                if (elapsed >= DIGIT_FADE_IN_DURATION_MS) {
                    Engine_DrawScaledAtlasSprite(sx + col * bw, sy + row * bw, bw, bw, 0, 0, 16, 16, 80, atlas);
                } else {
                    uint8_t alpha = (uint8_t)((elapsed * 100) / DIGIT_FADE_IN_DURATION_MS);
                    Engine_DrawScaledAtlasSpriteAlpha(sx + col * bw, sy + row * bw, bw, bw, 0, 0, 16, 16, 80, atlas, alpha);
                }
            }
            idx++;
        }
    }
}

void Digit_Update(uint32_t current_ticks, struct tm* tm_p) {
        // 更新金币砖块动画
        Digit_UpdateColonCoinAnimation(current_ticks);
    int target_time[4] = { tm_p->tm_hour / 10, tm_p->tm_hour % 10, tm_p->tm_min / 10, tm_p->tm_min % 10 };

    for (int i = 0; i < 4; i++) {
        if (digits[i].state == 0) {
            if (digits[i].val != target_time[i]) {
                digits[i].next_val = target_time[i];
                digits[i].state = 1; // 进入碎裂等待期
                digits[i].timer = current_ticks;
                ScheduleRandomShatter(i, current_ticks, digits[i].val);
            }
        } else if (digits[i].state == 1) {
            // 顺序碎裂旧数字：按计时器逐个触发碎屑，并保持旧数字显示
            for (int b = 0; b < DIGIT_BRICK_COUNT; b++) {
                if (bricks[i][b].state == 3 && current_ticks >= bricks[i][b].timer) {
                    bricks[i][b].state = 1;
                    bricks[i][b].timer = current_ticks;
                    Particle_SpawnBrickDebris(bricks[i][b].x + BRICK_W / 2, bricks[i][b].y + BRICK_W / 2);
                }
            }
            if (current_ticks - digits[i].timer > DIGIT_SHATTER_WAIT_MS) {
                digits[i].state = 2; // 碎片飞完，进入砖块渐显生成期
                digits[i].timer = current_ticks;
                digits[i].val = digits[i].next_val; // 更新为新数字值
                for (int b = 0; b < DIGIT_BRICK_COUNT; b++) {
                    int row = b / DIGIT_BRICK_COLS;
                    int col = b % DIGIT_BRICK_COLS;
                    if (font_3x5[digits[i].val][row] & (1 << (2 - col))) {
                        bricks[i][b].state = 2;
                        bricks[i][b].timer = 0;
                    }
                }
            }
        } else if (digits[i].state == 2) {
            bool all_visible = true;
            for (int b = 0; b < DIGIT_BRICK_COUNT; b++) {
                if (bricks[i][b].state == 2) {
                    int row = b / DIGIT_BRICK_COLS;
                    int col = b % DIGIT_BRICK_COLS;
                    int order = BRICK_ORDER(row, col);
                    uint32_t brick_start = digits[i].timer + order * DIGIT_BRICK_STAGGER_MS;
                    if (current_ticks >= brick_start + DIGIT_FADE_IN_DURATION_MS) {
                        bricks[i][b].state = 0;
                    } else {
                        all_visible = false;
                    }
                }
            }
            if (all_visible) {
                digits[i].state = 0; // 全部砖块生成完成，回到常亮
            }
        }

        // 更新砖块碎裂恢复计时器
        for (int b = 0; b < DIGIT_BRICK_COUNT; b++) {
            if (bricks[i][b].state == 1) { // 碎裂中
                if (current_ticks - bricks[i][b].timer > 3000) { // 3秒后恢复
                    bricks[i][b].state = 0;
                }
            }
        }
    }
}

void Digit_Render(uint32_t current_ticks, struct tm* tm_p, const uint8_t* atlas, const uint8_t* colon_atlas) {
    // 冒号 (背景最底层，三种状态轮播，使用金币 sprite_7_6_map)
    // 计算轮播状态 (0:状态1, 1:状态2, 2:状态3, 3:状态2, 4:状态1)
    uint32_t cycle_position = (current_ticks / COLON_CYCLE_INTERVAL_MS) % 5;
    uint8_t src_x = (cycle_position == 0 || cycle_position == 4) ? 0 : (cycle_position == 2) ? 32 : 16;
    // 渲染两个冒号金币砖块，支持动画
    for (int i = 0; i < COLON_COIN_COUNT; i++) {
        int y = colon_coins[i].y + colon_coins[i].anim_offset_y;
        Engine_DrawScaledAtlasSprite(colon_coins[i].x, y, BRICK_W, BRICK_W, src_x, 0, 16, 16, 80, colon_atlas);
    }

    // 4 个时间数字
    for (int i = 0; i < 4; i++) {
        if (digits[i].state == 0 || digits[i].state == 1) {
            DrawBrickDigit3x5(i, digits[i].sx, digits[i].sy, digits[i].val, atlas, current_ticks, UINT32_MAX);
        } else if (digits[i].state == 2) {
            DrawBrickDigit3x5(i, digits[i].sx, digits[i].sy, digits[i].val, atlas, current_ticks, digits[i].timer);
        }
    }
}


int Digit_CheckCollision(float mario_x, float mario_y, float mario_vx, float mario_vy, float* out_x, float* out_y) {
    *out_x = mario_x + mario_vx;
    *out_y = mario_y + mario_vy;

    float mario_left = *out_x;
    float mario_right = *out_x + 16;
    float mario_top = *out_y;
    float mario_bottom = *out_y + 16;

    // 先检查冒号金币砖块碰撞，确保可撞击且不会和马里奥重叠
    for (int i = 0; i < COLON_COIN_COUNT; i++) {
        ColonCoin* coin = &colon_coins[i];
        int coin_left = coin->x;
        int coin_right = coin->x + BRICK_W;
        int coin_top = coin->y + coin->anim_offset_y;
        int coin_bottom = coin_top + BRICK_W;

        if (mario_right > coin_left && mario_left < coin_right &&
            mario_bottom > coin_top && mario_top < coin_bottom) {
            float overlap_width = fminf(mario_right, coin_right) - fmaxf(mario_left, coin_left);
            float overlap_height = fminf(mario_bottom, coin_bottom) - fmaxf(mario_top, coin_top);

            if (overlap_width <= 0 || overlap_height <= 0) {
                continue;
            }

            if (coin->locked) {
                if (overlap_width < overlap_height) {
                    if (mario_x + 8.0f < coin_left + BRICK_W / 2.0f) {
                        *out_x = coin_left - 16;
                    } else {
                        *out_x = coin_right;
                    }
                    return 1;
                }

                if (mario_y + 8.0f < coin_top + BRICK_W / 2.0f) {
                    *out_y = coin_top - 16;
                    return 2;
                }

                *out_y = coin_bottom;
                return 3;
            }

            if (mario_vy < 0) {
                // 从下方顶到金币砖块：触发抖动 + 弹起
                *out_y = coin_bottom;
                mario_top = *out_y;
                mario_bottom = *out_y + 16;
                coin->state = COLON_COIN_STATE_UP;
                coin->timer = HAL_GetTicks();
                coin->anim_offset_y = 0;
                coin->locked = 1;
                coin->locked_until = coin->timer + COLON_COIN_UP_TIME + COLON_COIN_DOWN_TIME + 80;
                return 3;
            }

            // 其它方向的碰撞也强制分离，避免重叠
            if (overlap_width < overlap_height) {
                if (mario_x + 8.0f < coin_left + BRICK_W / 2.0f) {
                    *out_x = coin_left - 16;
                } else {
                    *out_x = coin_right;
                }
                return 1;
            }

            if (mario_y + 8.0f < coin_top + BRICK_W / 2.0f) {
                *out_y = coin_top - 16;
                return 2;
            }

            *out_y = coin_bottom;
            return 3;
        }
    }

    // 普通砖块逻辑保持不变
    for (int d = 0; d < 4; d++) {
        int digit = digits[d].val;
        int idx = 0;
        for (int row = 0; row < DIGIT_BRICK_ROWS; row++) {
            uint8_t line = font_3x5[digit][row];
            for (int col = 0; col < DIGIT_BRICK_COLS; col++) {
                if ((line & (1 << (2 - col))) && bricks[d][idx].state == 0) {
                    float brick_left = bricks[d][idx].x;
                    float brick_right = bricks[d][idx].x + BRICK_W;
                    float brick_top = bricks[d][idx].y;
                    float brick_bottom = bricks[d][idx].y + BRICK_W;

                    if (mario_right > brick_left && mario_left < brick_right &&
                        mario_bottom > brick_top && mario_top < brick_bottom) {

                        float overlap_width = fminf(mario_right, brick_right) - fmaxf(mario_left, brick_left);
                        float overlap_height = fminf(mario_bottom, brick_bottom) - fmaxf(mario_top, brick_top);

                        if (overlap_width <= 0 || overlap_height <= 0) {
                            idx++;
                            continue;
                        }

                        // 优先按最小重叠轴分离碰撞
                        if (overlap_width < overlap_height) {
                            if (mario_x + 8.0f < brick_left + BRICK_W / 2.0f) {
                                *out_x = brick_left - 16;
                            } else {
                                *out_x = brick_right;
                            }
                            return 1;
                        }

                        // 垂直碰撞：如果向上移动则顶碎砖块，否则从上方落到砖块上面
                        if (mario_vy < 0) {
                            *out_y = brick_bottom;
                            bricks[d][idx].state = 1;
                            bricks[d][idx].timer = HAL_GetTicks();
                            Particle_SpawnBrickDebris(bricks[d][idx].x + BRICK_W / 2, bricks[d][idx].y + BRICK_W / 2);
                            return 3;
                        }

                        *out_y = brick_top - 16;
                        return 2;
                    }
                }
                idx++;
            }
        }
    }

    return 0;
}
