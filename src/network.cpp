#ifdef ESP32S3_TARGET

#include "network.h"
#include <WiFiManager.h>
#include <WiFi.h>
#include <time.h>

// 全局变量
static uint32_t last_sync_ms = 0;
static bool sync_success = false;

// WiFiManager 实例
WiFiManager wm;

bool network_init(void) {
    // 设置自定义 AP 名称
    wm.setAPCallback([](WiFiManager *myWiFiManager) {
        Serial.println("Entered AP mode");
        Serial.print("AP SSID: ");
        Serial.println(myWiFiManager->getConfigPortalSSID());
    });
    
    // 设置连接超时（秒）
    wm.setConnectTimeout(30);
    wm.setConfigPortalTimeout(180); // 配置门户超时 3 分钟
    
    // 尝试自动连接已保存的 Wi‑Fi
    bool res = wm.autoConnect(WIFI_AP_NAME);
    if (!res) {
        Serial.println("Failed to connect and hit timeout");
        // 停留在 AP 模式，等待配网
        // 这里不会返回，因为 autoConnect 会阻塞直到配置成功或超时
        // 如果超时，会重启（根据 WiFiManager 默认行为）
        // 我们可以选择不重启，而是继续尝试
        // 但为了简化，我们让 WiFiManager 处理重启
        return false;
    } else {
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
}

bool ntp_sync(void) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, cannot sync NTP");
        return false;
    }
    
    Serial.println("Starting NTP synchronization...");
    
    // 配置时区偏移和夏令时
    configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    // 同时设置 TZ 环境变量，确保 localtime 等函数正确转换
    setenv("TZ", NTP_TIMEZONE, 1);
    tzset();
    
    // 等待时间同步（最长 10 秒）
    struct tm timeinfo;
    int retry = 0;
    while (retry < 20) {
        delay(500);
        if (getLocalTime(&timeinfo, 1000)) {
            char buffer[64];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
            Serial.printf("NTP sync successful: %s\n", buffer);
            last_sync_ms = millis();
            sync_success = true;
            return true;
        }
        retry++;
    }
    
    Serial.println("NTP sync failed");
    sync_success = false;
    return false;
}

bool ntp_need_sync(void) {
    if (!sync_success) {
        return true; // 从未同步过
    }
    uint32_t now = millis();
    // 处理毫秒溢出（约 50 天）
    if (now - last_sync_ms >= NTP_SYNC_INTERVAL_MS) {
        return true;
    }
    return false;
}

uint32_t ntp_last_sync(void) {
    return last_sync_ms;
}

void ntp_force_sync(void) {
    last_sync_ms = 0; // 强制下次检查时同步
}

void network_init_full(void) {
    Serial.begin(115200);
    Serial.println("MarioClock ESP32-S3 starting...");
    
    // 初始化 Wi‑Fi 连接（自动配网）
    if (!network_init()) {
        // 如果 network_init 返回 false，说明进入了 AP 模式并等待配网
        // WiFiManager 会阻塞直到配置成功，然后重启
        // 所以这里的代码可能不会执行
        Serial.println("Network initialization failed, restarting...");
        delay(3000);
        ESP.restart();
    }
    
    // 首次 NTP 同步
    if (ntp_sync()) {
        Serial.println("Initial NTP sync successful.");
    } else {
        Serial.println("Initial NTP sync failed, continuing with local time.");
    }
}

void network_periodic_task(void) {
    if (ntp_need_sync()) {
        Serial.println("Periodic NTP sync triggered.");
        if (ntp_sync()) {
            Serial.println("Periodic NTP sync successful.");
        } else {
            Serial.println("Periodic NTP sync failed.");
        }
    }
}

#endif // ESP32S3_TARGET