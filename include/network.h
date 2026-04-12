#pragma once

#ifdef ESP32S3_TARGET

#include <Arduino.h>

// 校准周期定义（毫秒）
// 默认 24 小时校准一次
#ifndef NTP_SYNC_INTERVAL_MS
#define NTP_SYNC_INTERVAL_MS (86400000UL)
#endif

// AP 热点名称
#ifndef WIFI_AP_NAME
#define WIFI_AP_NAME "ESP32_MarioClock"
#endif

// NTP 服务器（中国境内可用）
#ifndef NTP_SERVER
#define NTP_SERVER "ntp.aliyun.com"
#endif
#ifndef NTP_TIMEZONE
#define NTP_TIMEZONE "CST-8"
#endif
// 时区偏移（秒）
#ifndef NTP_GMT_OFFSET_SEC
#define NTP_GMT_OFFSET_SEC (8 * 3600) // 北京时间 UTC+8
#endif
#ifndef NTP_DAYLIGHT_OFFSET_SEC
#define NTP_DAYLIGHT_OFFSET_SEC 0 // 无夏令时
#endif

/**
 * @brief 初始化 WiFi 连接（自动配网）
 * @return true 连接成功，false 连接失败（进入 AP 模式）
 */
bool network_init(void);

/**
 * @brief 执行 NTP 时间同步
 * @return true 同步成功，false 同步失败
 */
bool ntp_sync(void);

/**
 * @brief 检查是否需要重新同步（基于校准周期）
 * @return true 需要同步，false 尚未到同步时间
 */
bool ntp_need_sync(void);

/**
 * @brief 获取上次同步的时间戳（毫秒）
 */
uint32_t ntp_last_sync(void);

/**
 * @brief 手动触发一次同步（用于测试或外部调用）
 */
void ntp_force_sync(void);

/**
 * @brief 高级网络初始化（包含串口、Wi‑Fi 自动配网、首次 NTP 同步）
 * @note 应在 setup() 中调用，失败时会重启设备
 */
void network_init_full(void);

/**
 * @brief 高级网络定期同步任务（检查并执行周期校准）
 * @note 应在 loop() 中循环调用
 */
void network_periodic_task(void);

#endif // ESP32S3_TARGET