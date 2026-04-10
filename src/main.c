// ============ 文件：src/main.c ============
// 主程序入口：任务调度器
// 
// 架构设计：
// - game.c 包含游戏核心逻辑和全局状态
// - mario.c 处理角色控制和动画
// - particle.c 管理粒子系统（碎屑效果）
// - digit.c 管理时间数字显示和状态机
// - main.c 仅负责任务调度和主循环，代码简洁清晰

#include "game.h"

int main(void) {
    // 初始化所有模块
    Game_Init();

    // 任务调度循环 (60 FPS)
    while (1) {
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

    return 0;
}