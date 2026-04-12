// ============ 文件：src/bluetooth_controller.cpp ============
// 蓝牙手柄控制器模块 (基于 Bluepad32)
// 提供 Xbox/PS4/Switch Pro 等蓝牙手柄支持，替代 GPIO 按键
//
// 依赖：Bluepad32 库 (通过 platform_packages 引入)
// 使用前需在 platformio.ini 中启用 HW_CONTROLLER_BLUEPAD32 宏

#ifdef HW_CONTROLLER_BLUEPAD32

#include "bluetooth_controller.h"
#include <Arduino.h>
#include <Bluepad32.h>
#include "hal.h"

// 全局手柄实例指针
static GamepadPtr myGamepad = nullptr;

// 手柄连接回调
static void onConnectedGamepad(GamepadPtr gp) {
    if (myGamepad == nullptr) {
        Serial.println("\n=================================");
        Serial.println("[Bluepad32] Bluetooth controller connected!");
        Serial.println("=================================\n");
        myGamepad = gp;
    }
}

// 手柄断开回调
static void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepad == gp) {
        Serial.println("[Bluepad32] Controller disconnected.");
        myGamepad = nullptr;
    }
}

void BluetoothController_Init(void) {
    Serial.println("Initializing Bluepad32 Bluetooth stack...");
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
    Serial.println("Bluetooth controller ready. Waiting for connection...");
}

uint8_t BluetoothController_GetInput(void) {
    // 必须持续调用 update 才能处理底层的蓝牙数据包
    BP32.update();

    uint8_t btn_state = 0;

    if (myGamepad && myGamepad->isConnected()) {
        // --- 映射方向键 ---
        // Bluepad32 原生位: 0x01=UP, 0x02=DOWN, 0x04=RIGHT, 0x08=LEFT
        if (myGamepad->dpad() & 0x08) btn_state |= BTN_LEFT;   // 左方向键 -> 左移
        if (myGamepad->dpad() & 0x04) btn_state |= BTN_RIGHT;  // 右方向键 -> 右移

        // --- 映射跳跃键 ---
        // 将 A、B、X 键都映射为跳跃，提供多种操作习惯
        if (myGamepad->a() || myGamepad->b() || myGamepad->x()) {
            btn_state |= BTN_JUMP;
        }
    }

    return btn_state;
}

#else // HW_CONTROLLER_BLUEPAD32 not defined

// 若未定义 HW_CONTROLLER_BLUEPAD32，则提供空实现，确保链接通过
#include "bluetooth_controller.h"
#include <Arduino.h>

void BluetoothController_Init(void) {
    // 无蓝牙手柄支持，不执行任何操作
}

uint8_t BluetoothController_GetInput(void) {
    return 0; // 无按键按下
}

#endif // HW_CONTROLLER_BLUEPAD32