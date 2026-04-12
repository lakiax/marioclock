# 代码重构说明 & 编译指南

## 🎯 重构概述
已将 main.c 从单体架构重构为**模块化架构**，显著提升了代码可读性和可维护性。

### 改进成果
- ✅ main.c 从 ~250 行精简到 ~50 行
- ✅ 建立清晰的**5层任务调度框架**
- ✅ 将混杂的功能分散到**4个独立模块**
- ✅ 完整的模块间依赖关系梳理

---

## 📦 新增模块说明

### 1. **mario.c/mario.h** - 角色控制模块
**职责**: 管理Mario的物理、输入、动画
- `Mario_Init()` - 初始化角色
- `Mario_UpdateInput()` - 处理按键输入
- `Mario_UpdatePhysics()` - 更新物理状态（重力、碰撞）
- `Mario_UpdateAnimation()` - 切换和更新动画帧
- `Mario_Render()` - 绘制角色

**所含常量**:
```c
GRAVITY, JUMP_FORCE, MOVE_ACCEL, MAX_SPEED, 
GROUND_Y, PIXELS_PER_FRAME
```

---

### 2. **particle.c/particle.h** - 粒子系统
**职责**: 管理碎屑粒子的生成、物理、渲染
- `Particle_Init()` - 初始化粒子池
- `Particle_SpawnBrickDebris(x, y)` - 产生单个砖块的4个碎屑
- `Particle_ShatterDigit(sx, sy, digit)` - 产生整个数字的碎屑
- `Particle_Update()` - 更新所有粒子的物理和生命周期
- `Particle_Render(atlas)` - 批量绘制活跃粒子

**特点**: 最多同屏150个粒子，自动回收超出屏幕的粒子

---

### 3. **digit.c/digit.h** - 数字显示模块
**职责**: 管理时间数字的显示状态和特效
- `Digit_Init()` - 初始化4个数字位
- `Digit_InitFromTime(tm_p, offset_y)` - 从系统时间初始化
- `Digit_Update(current_ticks, tm_p)` - 更新数字状态机
- `Digit_Render(current_ticks, tm_p, atlas, colon_atlas)` - 渲染数字和冒号

**状态机**:
```
常亮(0) ──时间变化→ 碎裂(1) ──1000ms→ 闪烁(2) ──600ms→ 常亮(0)
```

---

### 4. **game.c/game.h** - 游戏核心逻辑
**职责**: 管理全局游戏状态，协调各模块
- `Game_Init()` - 初始化所有子模块
- `Game_StartFrame()` - 记录帧开始时间
- `Game_TaskInput()` - 处理输入
- `Game_TaskPhysicsUpdate()` - 更新物理和动画
- `Game_TaskUpdateStateMachine()` - 更新状态机（数字+粒子）
- `Game_TaskRender()` - 分层渲染
- `Game_TaskFrameRateControl()` - 帧率控制（60FPS）

**全局状态结构** (内部使用):
```c
struct {
    Character mario;
    uint32_t current_ticks;
    uint32_t frame_start_ticks;
    time_t current_time;
    struct tm* tm_p;
} game_state;
```

---

## 🏗️ 重构后的架构

### 任务调度流程
```
每帧循环 (目标 60FPS)
│
├─ [1] 输入处理      Game_TaskInput()
│     └─ 读取按键 → 更新Mario速度和方向
│
├─ [2] 物理更新      Game_TaskPhysicsUpdate()
│     └─ 应用重力 → 更新位置 → 检测碰撞 → 更新动画
│
├─ [3] 状态机更新    Game_TaskUpdateStateMachine()
│     ├─ 检测时间变化 → 触发碎裂特效
│     ├─ 更新数字状态机（常亮→碎裂→闪烁）
│     └─ 更新粒子物理（重力、位置、生命周期）
│
├─ [4] 渲染          Game_TaskRender()
│     ├─ 清空缓冲区
│     ├─ 绘制冒号 (背景层)
│     ├─ 绘制数字 (中层)
│     ├─ 绘制粒子 (前中层)
│     ├─ 绘制Mario (前景层)
│     └─ 上传显示
│
└─ [5] 帧率控制      Game_TaskFrameRateControl()
      └─ 锁定 16ms/帧 (60 FPS)
```

### 模块依赖图
```
main.cpp (仅负责调度)
   ↓
game.c (协调各模块)
   ├─ mario.c
   ├─ particle.c
   └─ digit.c
      ↓
      engine.c (渲染引擎)
         ↓
         hal.c (硬件抽象)
```

---

## 🔧 编译配置

### 更新 platformio.ini
在 `[env:native]` 或相应环境下的 `build_src_dir` 中，确保包含所有源文件：

```ini
[env:native]
platform = native
build_src_filter = +<*> -<.git/> -<.svn/> -<CVS/>
; 上面这行默认会递归编译 src/ 中的所有 .c 文件
```

如果使用显式指定，添加：
```ini
build_src_filter = +<*> -<.git/> -<.svn/> -<CVS/> +<mario.c> +<particle.c> +<digit.c> +<game.c>
```

### 编译命令
```bash
pio run -e native     # 编译
pio run -e native -t upload  # 如果支持上传
```

---

## 📝 主程序 (main.cpp) 变化

### 重构前 (~250 行)
- 混杂的全局变量和常量
- 复杂的嵌套逻辑
- 难以理解和维护

### 重构后 (~50 行)
```c
int main(void) {
    Game_Init();  // 初始化

    while (1) {
        Game_StartFrame();                 // 记录时间
        Game_TaskInput();                  // Task 1: 输入
        Game_TaskPhysicsUpdate();          // Task 2: 物理
        Game_TaskUpdateStateMachine();     // Task 3: 状态机
        Game_TaskRender();                 // Task 4: 渲染
        Game_TaskFrameRateControl();       // Task 5: 帧率
    }
    return 0;
}
```

---

## 🎓 开发指南

### 添加新功能
假设要添加敌人系统：

1. 创建 `enemy.h` 和 `enemy.c`
2. 在 `game.h` 中声明 `Game_TaskEnemyUpdate()` 等
3. 在 `game.c` 中实现敌人管理逻辑
4. 在 `main.c` 的调度循环中插入敌人任务
5. 更新 platformio.ini

```c
// 在 main.cpp 中
while (1) {
    Game_StartFrame();
    Game_TaskInput();
    Game_TaskPhysicsUpdate();
    Game_TaskEnemyUpdate();       // 新增任务！
    Game_TaskUpdateStateMachine();
    Game_TaskRender();
    Game_TaskFrameRateControl();
}
```

### 修改任务优先级
仅需调整 `main.cpp` 中的任务顺序，无需修改其他模块。

### 调试技巧
在各任务间添加计时代码：
```c
uint32_t t1 = HAL_GetTicks();
Game_TaskPhysicsUpdate();
printf("Physics time: %d ms\n", HAL_GetTicks() - t1);
```

---

## ✅ 下一步

1. **编译验证**
   ```bash
   pio run -e native
   ```

2. **功能测试**
   - 验证Mario的移动、跳跃
   - 检查时间变化时的碎裂效果
   - 确认粒子物理效果

3. **性能分析**
   - 监测各任务运行时间
   - 确保帧率稳定在60FPS

4. **后续优化**
   - 如需添加音效、敌人等功能，按上述开发指南进行

---

## 📚 相关文档
- 详细架构说明：[ARCHITECTURE.md](ARCHITECTURE.md)
- 项目配置：[platformio.ini](platformio.ini)
