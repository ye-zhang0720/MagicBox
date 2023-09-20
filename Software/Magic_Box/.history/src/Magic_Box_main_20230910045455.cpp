/***************************************************
  Magic Box多功能固件源码
  （项目中若参考本工程源码，请注明参考来源）

  聚合多种APP，内置天气、时钟、相册、特效动画、软路由监控、浏览器文件修改。（各APP具体使用参考说明书）

  本程序参考：https://github.com/ClimbSnail/HoloCubic_AIO

  作者：西北偏北
  
  Last review/edit by ClimbSnail: 2023/08/24
 ****************************************************/

#include "driver/lv_port_indev.h"
#include "driver/lv_port_fs.h"

#include "common.h"
#include "sys/app_controller.h"

#include "app/app_conf.h"

#include <SPIFFS.h>
#include <esp32-hal.h>
#include <esp32-hal-timer.h>
#include <Wire.h>
#include "driver/sht3x.h"
#include "network.h"
#include "sys/sys_Time.h"
#include "sys/app_controller_gui.h"
#include "driver/backLight.h"

static bool isCheckAction = false;

/*** Component objects **7*/
ImuAction *act_info;           // 存放mpu6050返回的数据
AppController *app_controller; // APP控制器

TaskHandle_t handleTaskLvgl;
bool first_boot = false; // first boot flag

void get_BH1750_data();
void boot_msg_show_task_entry(void *parameter);

// void mam_task_entry(void *parameter);

TimerHandle_t xTimerAction = NULL;
void actionCheckHandle(TimerHandle_t xTimer)
{
    // 标志需要检测动作
    isCheckAction = true;
}

void my_print(const char *buf)
{
    Serial.printf("%s", buf);
    Serial.flush();
}

void sensor_task_entry(void *p)
{
    while (1)
    {
        sht3xGetData(); // we use here the maxWait option due fail save
        get_BH1750_data();
        auto_set_backLight();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);

    // 初始化运行时参数
    systemInfo = (System_information *)calloc(1, sizeof(System_information));
    systemInfo->TFFreeSpace = "无TF卡";
    strcpy(systemInfo->FrameVersion, SOFTWARE_VERSION);
    strcpy(systemInfo->HeadwareVersion, HARDWARE_VERSION);
    Serial.println(F("\nMagic Box version " SOFTWARE_VERSION "\n"));
    Serial.flush();
    // MAC ID可用作芯片唯一标识
    Serial.print(F("ChipID(EfuseMac): "));
    Serial.println(ESP.getEfuseMac());

    // 获取 Flash 大小（以字节为单位）
    systemInfo->flash_size = ESP.getFlashChipSize() / 1048576;
    Serial.printf("Flash Size: %dM\n", systemInfo->flash_size);

    app_controller = new AppController(); // APP控制器

    // 装载光线温湿度iic驱动
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);

    // flash运行模式
    // Serial.print(F("FlashChipMode: "));
    // Serial.println(ESP.getFlashChipMode());
    // Serial.println(F("FlashChipMode value: FM_QIO = 0, FM_QOUT = 1, FM_DIO = 2, FM_DOUT = 3, FM_FAST_READ = 4, FM_SLOW_READ = 5, FM_UNKNOWN = 255"));

    // 需要放在Setup里初始化
    if (!SPIFFS.begin(true, "/spiffs"))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    else
    {
        Serial.println(F("SPIFFS Mount Successed"));
    }

#ifdef PEAK
    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT);
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);
    /*电源使能保持*/
    Serial.println("Power: Waiting...");
    pinMode(CONFIG_POWER_EN_PIN, OUTPUT);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    digitalWrite(CONFIG_POWER_EN_PIN, HIGH);
    Serial.println("Power: ON");
    log_e("Power: ON");
#endif

    // config_read(NULL, &g_cfg);   // 旧的配置文件读取方式
    app_controller->read_config(&app_controller->sys_cfg);
    app_controller->read_config(&app_controller->mpu_cfg);
    app_controller->read_config(&app_controller->rgb_cfg);

    /*** Init screen ***/
    // app_controller->sys_cfg.rotation = 0;
    screen.init(app_controller->sys_cfg.rotation,
                app_controller->sys_cfg.backLight);

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.05).setRGB(0, 64, 64);

    /*** Init micro SD-Card ***/
    tf.init();

    Serial.println(F("SD Card Mount Successed"));

    lv_fs_fatfs_init();

    Serial.println(F("File System init Successed"));

#if LV_USE_LOG
    lv_log_register_print_cb(my_print);
#endif /*LV_USE_LOG*/

    app_controller->init();

    // 将APP"安装"到controller里
#if APP_WEATHER_USE
    app_controller->app_install(&weather_app);
#endif
#if APP_DigitalRain_USE
    app_controller->app_install(&DigitalRain_app);
#endif
#if APP_PICTURE_USE
    app_controller->app_install(&picture_app);
#endif
#if APP_MEDIA_PLAYER_USE
    app_controller->app_install(&media_app);
#endif
#if APP_SCREEN_SHARE_USE
    app_controller->app_install(&screen_share_app);
#endif
#if APP_FILE_MANAGER_USE
    app_controller->app_install(&file_manager_app);
#endif

    // app_controller->app_install(&server_app);

#if APP_IDEA_ANIM_USE
    app_controller->app_install(&idea_app);
#endif
#if APP_BILIBILI_FANS_USE
    app_controller->app_install(&bilibili_app);
#endif
#if APP_GAME_2048_USE
    app_controller->app_install(&game_2048_app);
#endif
#if APP_ANNIVERSARY_USE
    app_controller->app_install(&anniversary_app);
#endif
#if APP_HEARTBEAT_USE
    app_controller->app_install(&heartbeat_app, APP_TYPE_BACKGROUND);
#endif
#if APP_STOCK_MARKET_USE
    app_controller->app_install(&stockmarket_app);
#endif
#if APP_PC_RESOURCE_USE
    app_controller->app_install(&pc_resource_app);
#endif
#if APP_ROUTER_MONITOR_USE
    app_controller->app_install(&router_app);
#endif
#if APP_SETTING_USE
    app_controller->app_install(&settings_app);
#endif
#if APP_INFORMATION_USE
    app_controller->app_install(&information_app);
#endif
#if APP_OTA_USE
    app_controller->app_install(&ota_app);
#endif


    // 自启动APP
    app_controller->app_auto_start();

    screen.routine();

    // 优先显示屏幕 加快视觉上的开机时间
    // app_controller->main_process(&mpu.action_info);

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    Serial.println(F("App installing Successed"));

    /*** Init ambient-light sensor ***/
    lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE);
    SHT30_init();

    Serial.println(F("BH1750 & SHT30 inited"));

    mpu.init(app_controller->sys_cfg.mpu_order,
             app_controller->sys_cfg.auto_calibration_mpu,
             &app_controller->mpu_cfg); // 初始化比较耗时

    /*** 以此作为MPU6050初始化完成的标志 ***/
    RgbConfig *rgb_cfg = &app_controller->rgb_cfg;

    Serial.println(F("MPU6050 inited"));

    // 初始化RGB灯 HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            rgb_cfg->min_value_0, rgb_cfg->min_value_1, rgb_cfg->min_value_2,
                            rgb_cfg->max_value_0, rgb_cfg->max_value_1, rgb_cfg->max_value_2,
                            rgb_cfg->step_0, rgb_cfg->step_1, rgb_cfg->step_2,
                            rgb_cfg->min_brightness, rgb_cfg->max_brightness,
                            rgb_cfg->brightness_step, rgb_cfg->time};

    // 运行RGB任务
    set_rgb_and_run(&rgb_setting, RUN_MODE_TASK);

    // 先初始化一次动作数据 防空指针
    act_info = mpu.getAction();
    // 定义一个mpu6050的动作检测定时器
    xTimerAction = xTimerCreate("Action Check",
                                300 / portTICK_PERIOD_MS,
                                pdTRUE, (void *)0, actionCheckHandle);
    xTimerStart(xTimerAction, 0);

    // xTaskCreate(backgroud_task_entry, "backgroud_taskHandler", 1024, NULL, 2, NULL);
    // xTaskCreate(mam_task_entry, "mam_taskHandler", 1024, NULL, 2, NULL);
    delay(500);
    sys_rtc_init();
    xTaskCreate(sensor_task_entry, "sensor_taskHandler", 2 * 1024, NULL, 2, NULL);
    xTaskCreate(boot_msg_show_task_entry, "boot_msg_show_taskHandler", 4 * 1024, NULL, 2, NULL);
    // get_BH1750_data();

    Serial.println(F("Init completed"));
}

// void mam_task_entry(void *parameter)
// {
//     while (1)
//     {
//         Serial.println(ESP.getFreeHeap()/1024);
//         vTaskDelay(500);
//     }

// }
void boot_msg_show_task_entry(void *parameter)
{
    int count = 0;
    if (!first_boot)
    {
        AIO_LVGL_OPERATE_LOCK(change_loading_msg("陀螺仪校准完成!"););
        vTaskDelay(800);
        AIO_LVGL_OPERATE_LOCK(change_loading_msg("正在尝试连接WIFI..."););
        if (!app_controller->sys_cfg.ssid_0.isEmpty())
        {
            for (int i = 0; i < 10; i++)
            {
                if (WL_CONNECTED == WiFi.status())
                {
                    AIO_LVGL_OPERATE_LOCK(change_loading_msg("WIFI连接成功!"););
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    AIO_LVGL_OPERATE_LOCK(change_loading_msg("正在同步系统时间..."););
                    while ((!update_time_successed) && (count < 40))
                    {
                        count++;
                        vTaskDelay(500 / portTICK_PERIOD_MS);
                    }

                    if (update_time_successed)
                    {
                        AIO_LVGL_OPERATE_LOCK(change_loading_msg("时间同步成功!"););
                    }
                    else
                    {
                        AIO_LVGL_OPERATE_LOCK(change_loading_msg("时间同步失败!"););
                    }
                    first_boot = true;
                    vTaskDelete(NULL);
                    break;
                }
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
        AIO_LVGL_OPERATE_LOCK(change_loading_msg("WIFI连接失败, 请设置WIFI"););
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    first_boot = true;
    vTaskDelete(NULL);
}

void loop()
{
    screen.routine();

#ifdef PEAK
    if (!mpu.Encoder_GetIsPush())
    {
        Serial.println("mpu.Encoder_GetIsPush()1");
        delay(1000);
        if (!mpu.Encoder_GetIsPush())
        {
            Serial.println("mpu.Encoder_GetIsPush()2");
            // 适配Peak的关机功能
            digitalWrite(CONFIG_POWER_EN_PIN, LOW);
        }
    }
#endif

    if (isCheckAction)
    {
        isCheckAction = false;
        act_info = mpu.getAction();
    }
    app_controller->event_handle();
    if (first_boot)
        app_controller->main_process(act_info); // 运行当前进程
}