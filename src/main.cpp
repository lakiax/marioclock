// ============ 文件：src/main.cpp ============
// 主程序入口：任务调度器
//
// 架构设计：
// - game.c 包含游戏核心逻辑和全局状态
// - mario.c 处理角色控制和动画
// - particle.c 管理粒子系统（碎屑效果）
// - digit.c 管理时间数字显示和状态机
// - main.cpp 仅负责任务调度和主循环，代码简洁清晰
//
// 支持平台：
// - PC模拟器：使用标准main函数入口
// - ESP32-S3：使用Arduino框架setup/loop入口

#ifdef ESP32S3_TARGET
#include <Arduino.h>
#endif

// 游戏头文件用C方式链接
extern "C" {
#include "game.h"
}

// 游戏主循环函数（被main或loop调用）
static void Game_MainLoop(void) {
    // 任务调度循环 (60 FPS)
    // 记录帧开始时间
    Game_StartFrame();

    // =====================
    // 任务调度：优先级顺序
    // =====================

    // 1. 输入处理任务
    //    - 读取用户按键输入
    //    - 更新角色加速度和方向
    Game_TaskInput();

    // 2. 物理更新任务
    //    - 更新马里奥位置、速度、碰撞
    //    - 更新动画帧
    Game_TaskPhysicsUpdate();

    // 3. 状态机更新任务
    //    - 更新数字状态（常亮、碎裂、闪烁）
    //    - 更新粒子物理（重力、速度、生命周期）
    //    - 检测时间变化并触发碎裂特效
    Game_TaskUpdateStateMachine();

    // 4. 渲染任务
    //    - 清空屏幕缓冲区
    //    - 分层渲染：冒号 -> 数字 -> 粒子 -> 马里奥
    //    - 上传到屏幕驱动
    Game_TaskRender();

    // 5. 帧率控制任务
    //    - 锁定 60 FPS (16 ms/frame)
    Game_TaskFrameRateControl();
}

#ifdef ESP32S3_TARGET
// ================= ESP32-S3 Arduino 入口 =================

#include "network.h"

void setup() {
    network_init_full();
    // 初始化游戏模块
    Game_Init();
}

void loop() {
    network_periodic_task();
    Game_MainLoop();
}

#else
// ================= PC 模拟器标准入口 =================
int main(void) {
    // 初始化所有模块
    Game_Init();

    // 无限循环运行游戏
    while (1) {
        Game_MainLoop();
    }

    return 0;
}
#endif