# Mario Clock - 超级马里奥时钟

一个在 284x76 分辨率屏幕上显示的经典超级马里奥风格时钟程序。

## 项目概述

本项目是一个可爱的超级马里奥主题时钟，具有以下特点：
- 实时显示当前时间
- 马里奥角色可以在数字间行走、跳跃
- 数字碎裂动画效果
- 支持PC模拟器和ESP32-S3开发板两种平台

## 硬件支持

| 平台 | 屏幕 | 分辨率 | 说明 |
|------|------|--------|------|
| PC模拟器 | SDL2窗口 | 284x76 | 用于开发和测试 |
| ESP32-S3 | ST7789P3 TFT | 76x284 旋转为 284x76 | 实际硬件运行 |

### ESP32-S3 硬件连接

| TFT引脚 | ESP32-S3 GPIO |
|---------|---------------|
| MOSI    | GPIO 5        |
| SCLK    | GPIO 6        |
| CS      | GPIO 7        |
| DC      | GPIO 4        |
| RST     | GPIO 15       |

### 按键连接 (ESP32-S3)

| 功能 | GPIO | 连接方式 |
|------|------|----------|
| 左   | GPIO 10 | 低电平有效（接GND按下） |
| 右   | GPIO 11 | 低电平有效（接GND按下） |
| 跳跃 | GPIO 12 | 低电平有效（接GND按下） |

## 切换编译平台

本项目使用 PlatformIO 作为构建工具，支持两种编译目标：

### 方法1：修改默认环境（推荐）

编辑 [`platformio.ini`](platformio.ini) 文件，修改 `default_envs`：

```ini
[platformio]
; 编译PC模拟器版本
default_envs = pc_simulator

; 或编译ESP32-S3版本
default_envs = esp32s3
```

修改后保存文件，PlatformIO会自动切换到对应的环境。

### 方法2：使用命令行指定环境

无需修改配置文件，直接使用命令行参数：

```bash
# 编译并运行PC模拟器版本
pio run -e pc_simulator

# 编译ESP32-S3版本并上传
pio run -e esp32s3 --target upload

# 仅编译ESP32-S3版本
pio run -e esp32s3
```

### 方法3：VSCode状态栏切换

在VSCode底部状态栏：
1. 点击 PlatformIO 环境选择按钮（显示当前环境如 `pc_simulator`）
2. 从下拉菜单中选择目标环境
3. 点击编译或上传按钮

## 环境配置详情

### PC模拟器环境 (`pc_simulator`)

- **platform**: native
- **依赖**: SDL2库
- **编译宏**: `PC_SIMULATOR`
- **显示**: 窗口模式，支持DISPLAY_SCALE缩放

**Windows系统SDL2安装说明：**
1. 安装 [MSYS2](https://www.msys2.org/)
2. 打开MSYS2终端，执行：
   ```bash
   pacman -S mingw-w64-x86_64-SDL2
   ```
3. 将SDL2库路径添加到platformio.ini（已配置）

### ESP32-S3环境 (`esp32s3`)

- **platform**: espressif32@6.5.0
- **board**: esp32-s3-devkitc-1
- **framework**: arduino
- **编译宏**: `ESP32S3_TARGET`
- **屏幕**: ST7789P3 76x284（旋转后显示为284x76）

**首次使用ESP32-S3：**
1. 安装 [CP210x USB驱动](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
2. 连接开发板到电脑
3. 在platformio.ini中配置上传端口（可选）：
   ```ini
   [env:esp32s3]
   upload_port = COM3  ; Windows
   # 或
   upload_port = /dev/ttyUSB0  ; Linux
   ```

## 项目结构

```
marioclock/
├── include/           # 头文件
│   ├── hal.h         # 硬件抽象层接口
│   ├── engine.h      # 渲染引擎
│   ├── game.h        # 游戏逻辑
│   ├── mario.h       # 马里奥角色
│   ├── digit.h       # 数字显示
│   └── sprite.h      # 精灵定义
├── src/              # 源代码
│   ├── main.cpp      # 主程序入口（C++，支持双平台）
│   ├── hal_pc.c      # PC端HAL实现
│   ├── hal_esp32s3.cpp # ESP32-S3 HAL实现
│   ├── engine.c      # 渲染引擎实现
│   ├── game.c        # 游戏逻辑
│   ├── mario.c       # 马里奥角色控制
│   ├── digit.c       # 数字显示
│   └── particle.c    # 粒子效果
├── assert/           # 资源文件
│   └── sprites/      # 精灵图片数据
├── TFT_76_284/       # 参考：TFT屏幕测试例程
└── platformio.ini    # PlatformIO配置文件
```

## 编译和运行

### PC模拟器

```bash
# 编译
pio run -e pc_simulator

# 运行（Windows）
.pio/build/pc_simulator/program.exe

# 运行（Linux/Mac）
.pio/build/pc_simulator/program
```

### ESP32-S3

```bash
# 编译并上传
pio run -e esp32s3 --target upload

# 打开串口监视器
pio device monitor -e esp32s3
```

## 自定义配置

### 修改屏幕缩放（PC模拟器）

在 [`include/hal.h`](include/hal.h) 中修改：

```c
#define DISPLAY_SCALE 2  // 2倍放大
```

### 修改按键映射（PC模拟器）

在 [`src/hal_pc.c`](src/hal_pc.c) 中修改 `HAL_GetInput()` 函数。

### 修改屏幕参数

屏幕分辨率定义在 [`include/hal.h`](include/hal.h)：

```c
#define SCREEN_WIDTH 284
#define SCREEN_HEIGHT 76
```

## 开发注意事项

1. **HAL抽象层**: 添加新平台时只需实现 [`include/hal.h`](include/hal.h) 中的接口函数
2. **内存限制**: ESP32-S3版本需要注意RAM使用，screen_buffer占用约43KB
3. **屏幕旋转**: ESP32-S3使用ST7789的MV位实现90度旋转，无需软件旋转缓冲区
4. **性能优化**: ESP32-S3使用40MHz SPI和DMA批量传输优化显示速度

## 技术架构

项目采用分层架构设计：

```
┌─────────────────────────────────────┐
│           Application Layer          │
│  ┌─────────┐ ┌─────────┐ ┌────────┐ │
│  │  Mario  │ │  Digit  │ │Particle│ │
│  └─────────┘ └─────────┘ └────────┘ │
├─────────────────────────────────────┤
│           Game Logic Layer           │
│              game.c                  │
├─────────────────────────────────────┤
│          Engine Layer                │
│            engine.c                  │
│     (Sprite, Animation, Render)      │
├─────────────────────────────────────┤
│            HAL Layer                 │
│  ┌─────────┐      ┌──────────────┐  │
│  │ hal_pc.c│      │hal_esp32s3.cpp│  │
│  └─────────┘      └──────────────┘  │
├─────────────────────────────────────┤
│          Hardware Layer              │
│   PC + SDL2        ESP32-S3 + TFT   │
└─────────────────────────────────────┘
```

## 参考文档

- [架构设计说明](ARCHITECTURE.md)
- [重构指南](REFACTOR_GUIDE.md)
- [PlatformIO文档](https://docs.platformio.org/)
- [ESP32-Arduino文档](https://docs.espressif.com/projects/arduino-esp32/)

## 许可证

本项目仅供学习和个人使用。

---

**提示**: 如需查看TFT屏幕测试例程，请参考 [`TFT_76_284/src/main.cpp`](TFT_76_284/src/main.cpp)。
