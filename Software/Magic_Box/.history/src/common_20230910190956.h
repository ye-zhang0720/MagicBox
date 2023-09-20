#ifndef COMMON_H
#define COMMON_H

#define SOFTWARE_VERSION "2.4.2"
#define HARDWARE_VERSION "2.0.0"
#define GET_SYS_MILLIS xTaskGetTickCount // 获取系统毫秒数
// #define GET_SYS_MILLIS millis            // 获取系统毫秒数

#include "Arduino.h"
#include "driver/rgb_led.h"
#include "driver/flash_fs.h"
#include "driver/sd_card.h"
#include "driver/display.h"
#include "driver/imu.h"
#include "network.h"
#include <BH1750.h>

// SD_Card
#define SD_SCK 14
#define SD_MISO 16
#define SD_MOSI 15
#define SD_SS 13

// MUP6050
#define IMU_I2C_SDA 21
#define IMU_I2C_SCL 22

#define I2C_SDA 12
#define I2C_SCL 17

extern IMU mpu; // 原则上只提供给主程序调用
extern SdCard tf;
extern Pixel rgb;
// extern Config g_cfg;       // 全局配置文件
extern Network g_network;  // 网络连接
extern FlashFS g_flashCfg; // flash中的文件系统（替代原先的Preferences）
extern Display screen;     // 屏幕对象
extern BH1750 lightMeter;   // 光纤传感器对象
extern float lux;

struct System_information
{
    char FrameVersion[10];
    char HeadwareVersion[10];
    char *TFFreeSpace;
    uint8_t flash_size;
};
extern System_information *systemInfo;

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);


// 屏幕尺寸
#define SCREEN_HOR_RES 240 // 水平
#define SCREEN_VER_RES 240 // 竖直

// TFT屏幕接口
// #define PEAK
#ifdef PEAK
#define LCD_BL_PIN 12
/* Battery */
#define CONFIG_BAT_DET_PIN 37
#define CONFIG_BAT_CHG_DET_PIN 38
/* Power */
#define CONFIG_POWER_EN_PIN 21
#define CONFIG_ENCODER_PUSH_PIN 27
#else
#define LCD_BL_PIN 33
#endif

#define LCD_BL_PWM_CHANNEL 0

// 优先级定义(数值越小优先级越低)
// 最高为 configMAX_PRIORITIES-1
#define TASK_RGB_PRIORITY 0  // RGB的任务优先级
#define TASK_LVGL_PRIORITY 4 // LVGL的页面优先级

// lvgl 操作的锁
extern SemaphoreHandle_t lvgl_mutex;
// LVGL操作的安全宏（避免脏数据）
#define AIO_LVGL_OPERATE_LOCK(CODE)                          \
    if (pdTRUE == xSemaphoreTake(lvgl_mutex, portMAX_DELAY)) \
    {                                                        \
        CODE;                                                \
        xSemaphoreGive(lvgl_mutex);                          \
    }

struct SysUtilConfig
{
    String ssid_0;
    String password_0;
    String ssid_1;
    String password_1;
    String ssid_2;
    String password_2;
    String auto_start_app;        // 开机自启的APP名字
    uint8_t power_mode;           // 功耗模式（0为节能模式 1为性能模式）
    uint8_t backLight;            // 屏幕亮度（1-100）
    uint8_t rotation;             // 屏幕旋转方向
    uint8_t auto_calibration_mpu; // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
    uint8_t mpu_order;            // 操作方向
    uint8_t auto_backLight;       // 是否自动调节背光
};

#define GFX 0

#if GFX
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises
#include <Arduino_GFX_Library.h>
extern Arduino_HWSPI *bus;
extern Arduino_ST7789 *tft;

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
extern TFT_eSPI *tft;
extern TFT_eSPI scre;
#endif

#endif