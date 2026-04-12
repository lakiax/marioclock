#include <math.h>
#include "mario.h"
#include "hal.h"
#include "engine.h"
#include "mario_sprites.h"

// 动画定义
static const uint8_t* frames_idle[] = {sprite_1_2_map};
static const uint8_t* frames_run[]  = {sprite_1_10_map, sprite_1_16_map, sprite_1_20_map};
static const uint8_t* frames_skid[] = {sprite_1_28_map};
static const uint8_t* frames_jump[] = {sprite_1_34_map};

static Animation anim_idle = {frames_idle, 1, 1000};
static Animation anim_run  = {frames_run,  3, 0}; 
static Animation anim_skid = {frames_skid, 1, 1000};
static Animation anim_jump = {frames_jump, 1, 1000};

void Mario_Init(Character* mario, float x, float y) {
    mario->x = x;
    mario->y = y;
    mario->vx = 0;
    mario->vy = 0;
    mario->is_jumping = false;
    mario->facing_left = false;
    mario->is_skidding = false;
    mario->current_anim = &anim_idle;
    mario->anim_frame_idx = 0;
    mario->distance_moved = 0;
}

void Mario_UpdateInput(Character* mario, uint8_t btn) {
    bool left = (btn & BTN_LEFT) != 0;
    bool right = (btn & BTN_RIGHT) != 0;

    mario->is_skidding = false;

    if (!mario->is_jumping) {
        if (left && !right) {
            if (mario->vx > 0.1f) {
                mario->is_skidding = true;
                mario->vx -= MOVE_ACCEL * 2.0f;
                if (mario->vx < 0.0f) mario->vx = 0.0f;
            } else {
                mario->vx -= MOVE_ACCEL;
                mario->facing_left = true;
            }
        } else if (right && !left) {
            if (mario->vx < -0.1f) {
                mario->is_skidding = true;
                mario->vx += MOVE_ACCEL * 2.0f;
                if (mario->vx > 0.0f) mario->vx = 0.0f;
            } else {
                mario->vx += MOVE_ACCEL;
                mario->facing_left = false;
            }
        } else {
            mario->vx *= 0.85f;
        }
    } else {
        if (left && !right) {
            mario->vx -= MOVE_ACCEL;
            mario->facing_left = true;
        } else if (right && !left) {
            mario->vx += MOVE_ACCEL;
            mario->facing_left = false;
        } else {
            mario->vx *= 0.98f;
        }
    }

    if (mario->vx > MAX_SPEED) mario->vx = MAX_SPEED;
    if (mario->vx < -MAX_SPEED) mario->vx = -MAX_SPEED;

    if ((btn & BTN_JUMP) && !mario->is_jumping) {
        mario->vy = JUMP_FORCE;
        mario->is_jumping = true;
    }
}

void Mario_UpdatePhysics(Character* mario) {
    mario->vy += GRAVITY;
    
    // 限制跳跃高度：最高只能跳到屏幕顶部
    if (mario->y + mario->vy < 0) {
        mario->y = 0;
        mario->vy = 0;
    } else {
        mario->y += mario->vy;
    }
    
    mario->x += mario->vx;
    
    // 屏幕边界检查
    if (mario->x < 0) {
        mario->x = 0;
        mario->vx = 0;
    }
    if (mario->x > 284 - 16) { // 屏幕宽度284
        mario->x = 284 - 16;
        mario->vx = 0;
    }
    
    if (mario->y >= GROUND_Y) {
        mario->y = GROUND_Y;
        mario->vy = 0;
        mario->is_jumping = false;
    }
    
    // distance_moved现在在game.c中累积
}

void Mario_UpdateAnimation(Character* mario) {
    Animation* target_anim = &anim_idle;

    if (mario->is_jumping) {
        target_anim = &anim_jump;
    } else if (mario->is_skidding) {
        target_anim = &anim_skid;
    } else if (fabs(mario->vx) > 0.1f) {
        target_anim = &anim_run;
    } else {
        target_anim = &anim_idle;
    }

    if (mario->current_anim != target_anim) {
        mario->current_anim = target_anim;
        mario->anim_frame_idx = 0;
        mario->distance_moved = 0;
    }

    if (mario->current_anim == &anim_run && mario->distance_moved >= PIXELS_PER_FRAME) {
        mario->anim_frame_idx = (mario->anim_frame_idx + 1) % 3;
        mario->distance_moved -= PIXELS_PER_FRAME;
    }
}

void Mario_Render(const Character* mario) {
    Engine_DrawSprite((int)mario->x, (int)mario->y, 16, 16, 
        mario->current_anim->frames[mario->anim_frame_idx], mario->facing_left);
}
