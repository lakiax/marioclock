#ifndef DIGIT_H
#define DIGIT_H

#include <stdint.h>
#include <time.h>

// 数字显示参数
#define DIGIT_BRICK_ROWS 5                 // 3x5 字体竖排行数
#define DIGIT_BRICK_COLS 3                 // 3x5 字体横排列数
#define DIGIT_BRICK_COUNT (DIGIT_BRICK_ROWS * DIGIT_BRICK_COLS)
#define DIGIT_FADE_IN_DURATION_MS 180      // 每个砖块从透明到完全显示所需的时间
#define DIGIT_BRICK_STAGGER_MS 45          // 砖块逐个生成的间隔，从下到上、从右到左
#define DIGIT_SHATTER_STAGGER_MS 40        // 碎裂时砖块随机一个一个触发的间隔
#define DIGIT_SHATTER_WAIT_MS 1000         // 碎裂后等待多久再开始生成新数字
#define BRICK_W 10                         // 缩小砖块尺寸（原16）
#define COLON_X 137                        // 冒号X位置
#define COLON_CYCLE_INTERVAL_MS 200        // 冒号三种状态轮播间隔（毫秒）

typedef struct {
    int val;           // 当前显示的数字
    int next_val;      // 下一个目标数字（碎裂完成后生成新数字）
    int state;         // 0:常亮, 1:旧数字碎裂中, 2:生成中
    uint32_t timer;    // 状态计时器
    int sx, sy;        // 在屏幕上的位置
} TimeDigit;

typedef struct {
    int x, y;          // 砖块位置
    int state;         // 0:正常, 1:碎裂中, 2:待生成, 3:待碎裂
    uint32_t timer;    // 计时器/调度时间
} Brick;

/**
 * 初始化时间数字显示模块
 */
void Digit_Init(void);

/**
 * 从系统时间初始化 4 个数字
 * @param tm_p 系统时间
 * @param offset_y 垂直偏移量
 */
void Digit_InitFromTime(struct tm* tm_p, int offset_y);

/**
 * 更新所有数字的状态机
 * @param current_ticks 当前 tick 值
 * @param tm_p 当前系统时间
 */
void Digit_Update(uint32_t current_ticks, struct tm* tm_p);

/**
 * 渲染所有数字和冒号
 * @param current_ticks 当前 tick 值（用于冒号状态轮播）
 * @param tm_p 当前系统时间（保留参数，不再用于冒号）
 * @param atlas 数字图集数据指针
 * @param colon_atlas 冒号图集数据指针
 */
void Digit_Render(uint32_t current_ticks, struct tm* tm_p, const uint8_t* atlas, const uint8_t* colon_atlas);

/**
 * 检查马里奥与砖块的碰撞
 * @param mario_x 马里奥x位置
 * @param mario_y 马里奥y位置
 * @param mario_vx 马里奥x速度
 * @param mario_vy 马里奥y速度
 * @param out_x 输出调整后的x位置
 * @param out_y 输出调整后的y位置
 * @return 碰撞类型: 0=无碰撞, 1=水平碰撞, 2=从上方碰撞, 3=从下方碰撞(顶碎)
 */
int Digit_CheckCollision(float mario_x, float mario_y, float mario_vx, float mario_vy, float* out_x, float* out_y);

#endif // DIGIT_H
