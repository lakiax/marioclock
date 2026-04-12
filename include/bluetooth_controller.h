#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化蓝牙手柄控制器
 * 
 * 此函数初始化 Bluepad32 蓝牙栈并设置回调。
 * 必须在调用任何其他蓝牙手柄函数之前调用。
 */
void BluetoothController_Init(void);

/**
 * @brief 获取当前手柄输入状态
 * 
 * 返回一个位掩码，表示当前按下的按钮，符合 HAL 的按钮定义：
 * - BTN_LEFT  (1 << 0)
 * - BTN_RIGHT (1 << 1)
 * - BTN_JUMP  (1 << 2)
 * 
 * 映射规则：
 * - 左方向键 -> BTN_LEFT
 * - 右方向键 -> BTN_RIGHT
 * - A 按钮   -> BTN_JUMP
 * - B 按钮   -> BTN_JUMP (备用)
 * - X 按钮   -> BTN_JUMP (备用)
 * 
 * @return uint8_t 按钮状态位掩码
 */
uint8_t BluetoothController_GetInput(void);

#ifdef __cplusplus
}
#endif