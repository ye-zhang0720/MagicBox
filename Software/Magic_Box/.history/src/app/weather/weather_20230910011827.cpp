#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ArduinoJson.h"
#include <esp32-hal-timer.h>
#include <map>
#include "driver/sht3x.h"
#include "sys/sys_Time.h"

#define WEATHER_APP_NAME "天气时钟"
#define WEATHER_NOW_API "https://www.yiketianqi.com/free/day?appid=%s&appsecret=%s&unescape=1&city=%s"
#define WEATHER_NOW_API_UPDATE "https://yiketianqi.com/api?unescape=1&version=v6&appid=%s&appsecret=%s&city=%s"
#define WEATHER_DALIY_API "https://www.yiketianqi.com/free/week?unescape=1&appid=%s&appsecret=%s&city=%s"
#define WEATHER_PAGE_SIZE 2
#define UPDATE_WEATHER 0x01       // 更新天气

// 天气的持久化配置
#define WEATHER_CONFIG_PATH "/weather.cfg"

extern AppController *app_controller; // APP控制器

extern ESP32Time sys_rtc; // 用于时间解码

struct WT_Config
{
    String tianqi_appid;                // tianqiapid 的 appid
    String tianqi_appsecret;            // tianqiapid 的 appsecret
    String tianqi_addr;                 // tianqiapid 的地址（填中文）
    unsigned int weatherUpdataInterval; // 天气更新的时间间隔(h)
    unsigned int timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
};

struct WeatherAppRunData
{
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;
    unsigned int update_type; // 更新类型的标志位

    BaseType_t xReturned_task_task_update; // 更新数据的异步任务
    TaskHandle_t xHandle_task_task_update; // 更新数据的异步任务
    TimerHandle_t sensor_update_timer;     // 传感器更新定时器
    Weather wea;                           // 保存天气状况
};

static WT_Config cfg_data;
static WeatherAppRunData *run_data = NULL;

std::map<String, int> weatherMap = {{"qing", 0}, {"yin", 1}, {"yu", 2}, {"yun", 3}, {"bingbao", 4}, {"wu", 5}, {"shachen", 6}, {"lei", 7}, {"xue", 8}};

static void task_update(void *parameter); // 异步更新任务

static void write_config(WT_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->tianqi_appid + "\n";
    w_data = w_data + cfg->tianqi_appsecret + "\n";
    w_data = w_data + cfg->tianqi_addr + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->weatherUpdataInterval);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->timeUpdataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(WEATHER_CONFIG_PATH, w_data.c_str());
}

static void read_config(WT_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(WEATHER_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        // cfg->tianqi_addr = "杭州";
        // cfg->tianqi_appid = "81615486";
        // cfg->tianqi_appsecret = "hHxz9En2";
        // cfg->tianqi_addr = "萧山区";
        cfg->weatherUpdataInterval = 1;   // 天气更新的时间间隔1hour
        cfg->timeUpdataInterval = 900000; // 日期时钟更新的时间间隔900000(900s)
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[5] = {0};
        analyseParam(info, 5, param);
        cfg->tianqi_appid = param[0];
        cfg->tianqi_appsecret = param[1];
        cfg->tianqi_addr = param[2];
        cfg->weatherUpdataInterval = atol(param[3]);
        cfg->timeUpdataInterval = atol(param[4]);
    }
}

static int windLevelAnalyse(String str)
{
    int ret = 0;
    for (char ch : str)
    {
        if (ch >= '0' && ch <= '9')
        {
            ret = ret * 10 + (ch - '0');
        }
    }
    return ret;
}

static int get_weather(void)
{
    int res = 0;
    if (WL_CONNECTED != WiFi.status())
        return res;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    // snprintf(api, 128, WEATHER_NOW_API, cfg_data.tianqi_appid, cfg_data.tianqi_appsecret, cfg_data.tianqi_addr);
    snprintf(api, 128, WEATHER_NOW_API,
             cfg_data.tianqi_appid.c_str(),
             cfg_data.tianqi_appsecret.c_str(),
             cfg_data.tianqi_addr.c_str());

    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
                // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            strcpy(run_data->wea.cityname, sk["city"].as<String>().c_str());
            run_data->wea.weather_code = weatherMap[sk["wea_img"].as<String>()];
            run_data->wea.temperature = sk["tem"].as<int>();

            // 获取湿度
            run_data->wea.humidity = 50;
            char humidity[8] = {0};
            strncpy(humidity, sk["humidity"].as<String>().c_str(), 8);
            humidity[strlen(humidity) - 1] = 0; // 去除尾部的 % 号
            run_data->wea.humidity = atoi(humidity);

            run_data->wea.maxTemp = sk["tem_day"].as<int>();
            run_data->wea.minTemp = sk["tem_night"].as<int>();
            strcpy(run_data->wea.windDir, sk["win"].as<String>().c_str());
            run_data->wea.windLevel = windLevelAnalyse(sk["win_speed"].as<String>());
            run_data->wea.airQulity = airQulityLevel(sk["air"].as<int>());
            res = 1;
        }
        else
        {
            res = 2;
        }
    }
    else
    {
        res = 2;
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return res;
}

static long long get_timestamp()
{
    // // 使用本地的机器时钟
    // run_data->preNetTimestamp = run_data->preNetTimestamp + (GET_SYS_MILLIS() - run_data->preLocalTimestamp);
    run_data->preNetTimestamp = sys_rtc.getEpoch();
    run_data->preLocalTimestamp = GET_SYS_MILLIS();
    return run_data->preNetTimestamp;
}

static int get_daliyWeather()
{
    int res = 0;
    if (WL_CONNECTED != WiFi.status())
        return res;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    snprintf(api, 128, WEATHER_DALIY_API,
             cfg_data.tianqi_appid.c_str(),
             cfg_data.tianqi_appsecret.c_str(),
             cfg_data.tianqi_addr.c_str());

    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            for (int gDW_i = 0; gDW_i < 3; ++gDW_i)
            {
                run_data->wea.daily_max[gDW_i] = sk["data"][gDW_i + 1]["tem_day"].as<int>();
                run_data->wea.daily_min[gDW_i] = sk["data"][gDW_i + 1]["tem_night"].as<int>();
                run_data->wea.daily_weather_code[gDW_i] = weatherMap[sk["data"][gDW_i + 1]["wea_img"].as<String>()];
            }
            strcpy(run_data->wea.day1_windDir, sk["data"][1]["win"].as<String>().c_str());
            strcpy(run_data->wea.day2_windDir, sk["data"][2]["win"].as<String>().c_str());
            strcpy(run_data->wea.day3_windDir, sk["data"][3]["win"].as<String>().c_str());
            strcpy(run_data->wea.day1_Date, sk["data"][1]["date"].as<String>().substring(5, 10).c_str());
            strcpy(run_data->wea.day2_Date, sk["data"][2]["date"].as<String>().substring(5, 10).c_str());
            strcpy(run_data->wea.day3_Date, sk["data"][3]["date"].as<String>().substring(5, 10).c_str());
            strcpy(run_data->wea.day1_Wea, sk["data"][1]["wea"].as<String>().c_str());
            strcpy(run_data->wea.day2_Wea, sk["data"][2]["wea"].as<String>().c_str());
            strcpy(run_data->wea.day3_Wea, sk["data"][3]["wea"].as<String>().c_str());
            // Serial.printf(run_data->wea.day1_Data);
            res = 1;
        }
        else
        {
            res = 2;
        }
    }
    else
    {
        res = 2;
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return res;
}

static void UpdateTime_RTC(long long timestamp)
{
    struct TimeStr t;
    t.month = sys_rtc.getMonth() + 1;
    t.day = sys_rtc.getDay();
    t.hour = sys_rtc.getHour(true);
    t.minute = sys_rtc.getMinute();
    t.second = sys_rtc.getSecond();
    t.weekday = sys_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    AIO_LVGL_OPERATE_LOCK(display_time(t, LV_SCR_LOAD_ANIM_NONE););
}

void update_sensor_data_callback(void *p)
{
    if (run_data->clock_page == 0)
    {
        run_data->wea.indoor_temperature = getTemperatire();
        run_data->wea.indoor_huimdity = getHumidity() + 0.5;
        
        // Serial.print(run_data->wea.indoor_temperature);
        // Serial.print(" H: ");
        // Serial.print(run_data->wea.indoor_huimdity);
        // Serial.println("");
    }
}

static int weather_init(AppController *sys)
{
    tft->setSwapBytes(true);
    weather_gui_init();
    // 获取配置信息
    read_config(&cfg_data);

    // 初始化运行时参数
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    memset((char *)&run_data->wea, 0, sizeof(Weather));
    run_data->preNetTimestamp = 1577808000000;      // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->preLocalTimestamp = GET_SYS_MILLIS(); // 上一次的本地机器时间戳
    run_data->clock_page = 0;
    // 强制更新
    run_data->coactusUpdateFlag = 0x01;
    run_data->update_type = 0x00; // 表示什么也不需要更新
    run_data->wea.indoor_temperature = getTemperatire();
    run_data->wea.indoor_huimdity = getHumidity() + 0.5;
    run_data->wea.update_weather_flag = 3;
    run_data->wea.update_3day_weather_flag = 3;
    run_data->wea.weather_code = 99;
    for (uint8_t i = 0; i < 3; i++)
    {
        run_data->wea.daily_weather_code[i] = 99;
    }
    strcpy(run_data->wea.cityname, cfg_data.tianqi_addr.c_str());

    // AIO_LVGL_OPERATE_LOCK(display_weather_init(run_data->wea, LV_SCR_LOAD_ANIM_FADE_ON););
    // 目前更新数据的任务栈大小5000够用，4000不够用
    // 为了后期迭代新功能 当前设置为8000

    run_data->xReturned_task_task_update = xTaskCreate(
        task_update,                          /*任务函数*/
        "Task_update",                        /*带任务名称的字符串*/
        6 * 1024,                             /*堆栈大小，单位为字节*/
        NULL,                                 /*作为任务输入传递的参数*/
        2,                                    /*任务的优先级*/
        &run_data->xHandle_task_task_update); /*任务句柄*/
    run_data->sensor_update_timer = xTimerCreate("sensor_update", 2000 / portTICK_PERIOD_MS, pdTRUE, NULL, update_sensor_data_callback);
    xTimerStart(run_data->sensor_update_timer, 0);
    return 0;
}

static void weather_process(AppController *sys,
                            const ImuAction *act_info)
{

    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }
    else if (GO_FORWORD == act_info->active)
    {
        // 间接强制更新
        run_data->coactusUpdateFlag = 0x01;
        // vTaskDelay(500);
        delay(500); // 以防间接强制更新后，生产很多请求 使显示卡顿
    }
    else if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        run_data->clock_page = (run_data->clock_page + 1) % WEATHER_PAGE_SIZE;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        // run_data->clock_page = (run_data->clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        run_data->clock_page = (run_data->clock_page + 1) % WEATHER_PAGE_SIZE;
    }

    // 界面刷新
    if (run_data->clock_page == 0)
    {
        AIO_LVGL_OPERATE_LOCK(display_weather_init(run_data->wea, anim_type););
        AIO_LVGL_OPERATE_LOCK(display_space(););
        AIO_LVGL_OPERATE_LOCK(update_indoor_temp_humi(run_data->wea.indoor_temperature, run_data->wea.indoor_huimdity););
        vTaskDelay(30);
    }
    else if (run_data->clock_page == 1)               
    {

        // 仅在切换界面时获取一次未来天气
        AIO_LVGL_OPERATE_LOCK(display_3Days_weather_init(run_data->wea, anim_type););
        vTaskDelay(300);
    }
    UpdateTime_RTC(get_timestamp());
}

static void weather_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int weather_exit_callback(void *param)
{

    // 查杀异步任务
    if (run_data->xReturned_task_task_update == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_task_update);
    }

    xSemaphoreGive(lvgl_mutex);
    weather_gui_del();

    // vTaskDelete(run_data->xHandle_task_task_update);
    xTimerDelete(run_data->sensor_update_timer, 0);
    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}
static void task_update(void *parameter)
{
    long count = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (1)
    {
        // Serial.println("update temp humi");
        // update_indoor_temp_humi(15, 55, LV_SCR_LOAD_ANIM_NONE);
        if ((count % (3600 * cfg_data.weatherUpdataInterval) == 0) || (run_data->coactusUpdateFlag == 0x01)) // 每1小时执行
        {
            setCpuFrequencyMhz(240);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            Serial.printf("updating weather data\n");
            run_data->coactusUpdateFlag = 0x0;
            if ((run_data->wea.update_weather_flag == 1) && (run_data->wea.update_3day_weather_flag == 1))
            {
                run_data->wea.update_weather_flag = 3;
                run_data->wea.update_3day_weather_flag = 3;
                AIO_LVGL_OPERATE_LOCK(update_weather_info(run_data->wea, 3));
            }

            app_controller->send_to(WEATHER_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, NULL, NULL);

            for (int i = 0; i < 20; i++)
            {
                if (WL_CONNECTED == WiFi.status())
                    break;
                vTaskDelay(500);
            }

            if (run_data->wea.update_weather_flag != 1)
            {
                run_data->wea.update_weather_flag = get_weather();
                vTaskDelay(500);
            }

            if (run_data->wea.update_3day_weather_flag != 1)
            {
                run_data->wea.update_3day_weather_flag = get_daliyWeather();
            }

            if ((run_data->wea.update_weather_flag == 0) || (run_data->wea.update_3day_weather_flag == 0))
            {
                AIO_LVGL_OPERATE_LOCK(update_weather_info(run_data->wea, 0));
                run_data->coactusUpdateFlag = 0x01;
                vTaskDelay(60000 / portTICK_PERIOD_MS);
            }

            if ((run_data->wea.update_weather_flag == 1) || (run_data->wea.update_3day_weather_flag == 1))
            {

                AIO_LVGL_OPERATE_LOCK(update_weather_info(run_data->wea, 1));
                run_data->coactusUpdateFlag = 0;
                count = 0;
            }

            if ((run_data->wea.update_weather_flag == 2) || (run_data->wea.update_3day_weather_flag == 2))
            {
                AIO_LVGL_OPERATE_LOCK(update_weather_info(run_data->wea, 2));
                run_data->coactusUpdateFlag = 0x01;
                vTaskDelay(60000 / portTICK_PERIOD_MS);
            }
            setCpuFrequencyMhz(160);
        }
        count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static void weather_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "tianqi_appid"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_appid.c_str());
        }
        else if (!strcmp(param_key, "tianqi_appsecret"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_appsecret.c_str());
        }
        else if (!strcmp(param_key, "tianqi_addr"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_addr.c_str());
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%du", cfg_data.weatherUpdataInterval);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%du", cfg_data.timeUpdataInterval);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "tianqi_appid"))
        {
            cfg_data.tianqi_appid = param_val;
        }
        else if (!strcmp(param_key, "tianqi_appsecret"))
        {
            cfg_data.tianqi_appsecret = param_val;
        }
        else if (!strcmp(param_key, "tianqi_addr"))
        {
            cfg_data.tianqi_addr = param_val;
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            cfg_data.weatherUpdataInterval = atol(param_val);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            cfg_data.timeUpdataInterval = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ weather_app = {WEATHER_APP_NAME, &app_weather, "",
                       weather_init, weather_process, weather_background_task,
                       weather_exit_callback, weather_message_handle};
