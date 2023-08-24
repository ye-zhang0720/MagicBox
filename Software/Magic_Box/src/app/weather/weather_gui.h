#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H


struct Weather
{
    int weather_code; // 天气现象代码
    int temperature;  // 温度
    int humidity;     // 湿度
    int maxTemp;      // 最高气温
    int minTemp;      // 最低气温
    char windDir[20];
    char cityname[10]; // 城市名
    int windLevel;
    int airQulity;

    int indoor_temperature; //当前室内温度
    int indoor_huimdity;    //当前室内湿度

    short daily_max[3];
    short daily_min[3];
    int daily_weather_code[3]; // 天气现象代码
    char day1_windDir[20];
    char day2_windDir[20];
    char day3_windDir[20];
    char day1_Date[20];
    char day2_Date[20];
    char day3_Date[20];
    char day1_Wea[40];
    char day2_Wea[40];
    char day3_Wea[40];
    unsigned char update_weather_flag;         //0:网络连接失败 1:更新完成 2:获取信息失败
    unsigned char update_3day_weather_flag;
};


struct TimeStr
{
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;
};

enum
{
    WEATHER_MAIN_TEMP,
    WEATHER_MAIN_HUMI,
    DATA,
    DATA_DAY1,
    DATA_DAY2,
    DATA_DAY3,
    TEMP_DAY1,
    TEMP_DAY2,
    TEMP_DAY3,
    WIND_DAY1,
    WIND_DAY2,
    WIND_DAY3,
    WEATHER_IMG_DAY1,
    WEATHER_IMG_DAY2,
    WEATHER_IMG_DAY3,
    WEATHER_IMG,
    WEA_DAY1,
    WEA_DAY2,
    WEA_DAY3,
    WEATHER_MAIN_TIME,
    WEATHER_MAIN_TIME_SEC,
    DATA_LABEL,
    CITY_NAME,
    AIR_QUALITY,
    TXTLABEL,
};


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    void weather_gui_init(void);
    void display_curve_init(lv_scr_load_anim_t anim_type);
    void display_curve(short maxT[], short minT[], lv_scr_load_anim_t anim_type);
    void display_weather_init(struct Weather weaInfo, lv_scr_load_anim_t anim_type);
    // void display_weather(struct Weather weaInfo, lv_scr_load_anim_t anim_type);
    void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type);
    void weather_gui_release();
    void weather_gui_del(void);
    void display_space(void);
    int airQulityLevel(int q);
    void update_indoor_temp_humi(int temp, int humi);
    static void label_event_cb(lv_event_t* e);
    static void bar_event_cb(lv_event_t* e);
    static void img_event_cb(lv_event_t *e);
    void display_3Days_weather_init(struct Weather weaInfo, lv_scr_load_anim_t anim_type);
    // void display_3Days_weather(struct Weather weaInfo, lv_scr_load_anim_t anim_type);
    void update_weather_info(struct Weather weaInfo, char data_code);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_weather;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif