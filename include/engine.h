// ============ 文件：include/engine.h ============
#pragma once
#include <stdint.h>
#include <stdbool.h>

// 动画序列定义
typedef struct {
    const uint8_t** frames; // 帧数组指针 (例如: {跑1, 跑2, 跑3})
    uint8_t frame_count;    // 帧数
    uint16_t delay_ms;      // 帧间隔时间
} Animation;

// 角色对象定义
typedef struct {
    float x, y;             // 浮点数坐标，为了更平滑的物理计算
    float vx, vy;           // X/Y 速度
    bool is_jumping;        // 是否在空中
    bool facing_left;       // 是否朝左 (用于镜像画图)
    bool is_skidding;       // 是否在水平反向按键时紧急刹停
    Animation* current_anim;// 当前播放的动画
    uint8_t anim_frame_idx; // 播到第几帧了
    uint32_t last_anim_time;// 上次换帧时间
    float distance_moved;   // 【新增】计步器：累计走过的像素距离
} Character;

// 清屏与画图接口
void Engine_ClearBuffer(uint16_t color);
void Engine_DrawSprite(int sx, int sy, int sw, int sh, const uint8_t* img_data, bool flip_h);
// 从大图集(Atlas)中裁剪并绘制 16x16 的切片
void Engine_DrawAtlasSprite(int sx, int sy, int sw, int sh, int src_x, int src_y, int atlas_w, const uint8_t* img_data);
// 从大图集(Atlas)中裁剪并缩放绘制（将 sw×sh 源区域缩放到 dw×dh 目标尺寸）
void Engine_DrawScaledAtlasSprite(int sx, int sy, int dw, int dh, int src_x, int src_y, int sw, int sh, int atlas_w, const uint8_t* img_data);
// 缩放绘制并支持透明度渐显，alpha 为 0~100 的百分比
void Engine_DrawScaledAtlasSpriteAlpha(int sx, int sy, int dw, int dh, int src_x, int src_y, int sw, int sh, int atlas_w, const uint8_t* img_data, uint8_t alpha);
extern uint16_t screen_buffer[];

// 3x5 数字字体渲染
void Engine_DrawNumber(int x, int y, int number, uint16_t color);