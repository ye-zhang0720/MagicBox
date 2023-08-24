#include "weather_gui.h"
#include "weather_image.h"

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(lv_font_ibmplex_64);
LV_FONT_DECLARE(weiruanyahei_27);

static lv_style_t default_style;
static lv_style_t chFont_style;
static lv_style_t numberSmall_style;
static lv_style_t numberBig_style;
static lv_style_t btn_style;
static lv_style_t bar_style;
static lv_style_t label_cont_style;
static lv_style_t icon_cont_style;

static lv_obj_t *scr_1 = NULL;
static lv_obj_t *scr_2 = NULL;

static lv_obj_t *spaceImg = NULL;

// 太空人图标路径的映射关系
const void *manImage_map[] = {&man_0, &man_1, &man_2, &man_3, &man_4, &man_5, &man_6, &man_7, &man_8, &man_9};
static const char weekDayCh[7][4] = {"日", "一", "二", "三", "四", "五", "六"};
static const char airQualityCh[6][10] = {"优", "良", "轻度", "中度", "重度", "严重"};

void update_weather_info(struct Weather weaInfo, char data_code)
{
    if (scr_1 != NULL)
    {
        switch (data_code)
        {
        case 0:
            lv_msg_send(TXTLABEL, "网络未连接, 请连接网络!");
            break;
        case 1:
            lv_msg_send(WEATHER_IMG, &weaInfo.weather_code);
            lv_msg_send(CITY_NAME, weaInfo.cityname);
            lv_msg_send(AIR_QUALITY, airQualityCh[weaInfo.airQulity]);
            char buf[60] = "";
            sprintf(buf, "最低气温%d°C, 最高气温%d°C, %s%d 级.   ", weaInfo.minTemp, weaInfo.maxTemp, weaInfo.windDir, weaInfo.windLevel);
            lv_msg_send(TXTLABEL, buf);
            break;
        case 2:
            lv_msg_send(TXTLABEL, "天气更新失败!");
            break;
        case 3:
            lv_msg_send(TXTLABEL, "正在获取天气数据!");
            break;

        default:
            break;
        }
    }
    else if ((scr_2 != NULL) && (data_code == 1))
    {
        char buf[20] = "";
        lv_msg_send(DATA_DAY1, weaInfo.day1_Date);
        lv_msg_send(DATA_DAY2, weaInfo.day2_Date);
        lv_msg_send(DATA_DAY3, weaInfo.day3_Date);

        lv_msg_send(WIND_DAY1, weaInfo.day1_windDir);
        lv_msg_send(WIND_DAY2, weaInfo.day2_windDir);
        lv_msg_send(WIND_DAY3, weaInfo.day3_windDir);

        sprintf(buf, "%2d~%2d°", weaInfo.daily_min[0], weaInfo.daily_max[0]);
        lv_msg_send(TEMP_DAY1, buf);
        sprintf(buf, "%2d~%2d°", weaInfo.daily_min[1], weaInfo.daily_max[1]);
        lv_msg_send(TEMP_DAY2, buf);
        sprintf(buf, "%2d~%2d°", weaInfo.daily_min[2], weaInfo.daily_max[2]);
        lv_msg_send(TEMP_DAY3, buf);

        lv_msg_send(WEA_DAY1, weaInfo.day1_Wea);
        lv_msg_send(WEA_DAY2, weaInfo.day2_Wea);
        lv_msg_send(WEA_DAY3, weaInfo.day3_Wea);
        lv_msg_send(WEATHER_IMG_DAY1, &weaInfo.daily_weather_code[0]);
        lv_msg_send(WEATHER_IMG_DAY2, &weaInfo.daily_weather_code[1]);
        lv_msg_send(WEATHER_IMG_DAY3, &weaInfo.daily_weather_code[2]);
    }
}

void weather_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));
    // lv_style_set_width(&default_style, 240);
    // lv_style_set_height(&default_style, 240);

    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &weiruanyahei_27);

    lv_style_init(&numberSmall_style);
    lv_style_set_text_opa(&numberSmall_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberSmall_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberSmall_style, &lv_font_ibmplex_64);

    lv_style_init(&numberBig_style);
    lv_style_set_text_opa(&numberBig_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBig_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberBig_style, &lv_font_ibmplex_115);

    lv_style_init(&btn_style);
    lv_style_set_border_width(&btn_style, 0);
    lv_style_init(&bar_style);
    lv_style_set_bg_color(&bar_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&bar_style, 2);
    lv_style_set_border_color(&bar_style, lv_color_hex(0xFFFFFF));
    lv_style_set_pad_top(&bar_style, 1); // 指示器到背景四周的距离
    lv_style_set_pad_bottom(&bar_style, 1);
    lv_style_set_pad_left(&bar_style, 1);
    lv_style_set_pad_right(&bar_style, 1);

    lv_style_init(&label_cont_style);
    lv_style_set_bg_color(&label_cont_style, lv_color_hex(0x000000));
    lv_style_set_border_side(&label_cont_style, LV_BORDER_SIDE_NONE);
    lv_style_set_width(&label_cont_style, 80);
    lv_style_set_height(&label_cont_style, 30);

    lv_style_init(&icon_cont_style);
    lv_style_set_bg_color(&icon_cont_style, lv_color_hex(0x000000));
    lv_style_set_border_side(&icon_cont_style, LV_BORDER_SIDE_NONE);
    lv_style_set_width(&icon_cont_style, 60);
    lv_style_set_height(&icon_cont_style, 60);

    // scr_1 = lv_obj_create(NULL);
    // lv_obj_add_style(scr_1, &default_style, LV_STATE_DEFAULT);

    // scr_2 = lv_obj_create(NULL);
    // lv_obj_add_style(scr_2, &default_style, LV_STATE_DEFAULT);
}

void display_3Days_weather_init(struct Weather weaInfo, lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == scr_2)
        return;

    weather_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr_2 = lv_obj_create(NULL);
    lv_obj_add_style(scr_2, &default_style, LV_STATE_DEFAULT);

    char buf[20] = "";
    
    // top data display
    lv_obj_t *daily_dateLabel = lv_label_create(scr_2);
    lv_obj_add_style(daily_dateLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_obj_align(daily_dateLabel, LV_ALIGN_TOP_MID, 0, 10);

    lv_msg_subsribe_obj(DATA, daily_dateLabel, "%s");
    lv_obj_add_event_cb(daily_dateLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(DATA, "01月01日  周一  00:00");
    // day 1
    lv_obj_t *data_d1_cont = lv_obj_create(scr_2);
    lv_obj_add_style(data_d1_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(data_d1_cont, LV_ALIGN_TOP_LEFT, 0, 40);
    lv_obj_clear_flag(data_d1_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(data_d1_cont, LV_DIR_HOR);

    lv_obj_t *date1Label = lv_label_create(data_d1_cont);
    lv_obj_add_style(date1Label, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(DATA_DAY1, date1Label, "%s");
    lv_obj_align(date1Label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(date1Label, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(DATA_DAY1, weaInfo.day1_Date);

    lv_obj_t *weather_icon_d1_cont = lv_obj_create(scr_2);
    lv_obj_add_style(weather_icon_d1_cont, &icon_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(weather_icon_d1_cont, LV_ALIGN_TOP_LEFT, 10, 80);
    lv_obj_clear_flag(weather_icon_d1_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(weather_icon_d1_cont, LV_DIR_HOR);

    lv_obj_t *day1_weather_Img = lv_img_create(weather_icon_d1_cont);
    lv_img_set_src(day1_weather_Img, "A:99@1x.png");
    lv_obj_align(day1_weather_Img, LV_ALIGN_CENTER, 0, 0);
    lv_msg_subsribe_obj(WEATHER_IMG_DAY1, day1_weather_Img, "A:%d@1x.png");
    lv_obj_add_event_cb(day1_weather_Img, img_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEATHER_IMG_DAY1, &weaInfo.daily_weather_code[0]);

    lv_obj_t *temp_d1_cont = lv_obj_create(scr_2);
    lv_obj_add_style(temp_d1_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(temp_d1_cont, LV_ALIGN_TOP_LEFT, 0, 150);
    lv_obj_clear_flag(temp_d1_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(temp_d1_cont, LV_DIR_HOR);

    lv_obj_t *day1_temp = lv_label_create(temp_d1_cont);
    lv_obj_add_style(day1_temp, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(TEMP_DAY1, day1_temp, "%s");
    lv_obj_align(day1_temp, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(day1_temp, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    sprintf(buf, "%2d~%2d°", weaInfo.daily_min[0], weaInfo.daily_max[0]);
    lv_msg_send(TEMP_DAY1, buf);

    lv_obj_t *wind_d1_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wind_d1_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wind_d1_cont, LV_ALIGN_TOP_LEFT, 0, 180);
    lv_obj_clear_flag(wind_d1_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wind_d1_cont, LV_DIR_HOR);

    lv_obj_t *day1_wind = lv_label_create(wind_d1_cont);
    lv_obj_add_style(day1_wind, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day1_wind, 80, 30);
    lv_label_set_long_mode(day1_wind, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day1_wind, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WIND_DAY1, day1_wind, "%s");
    lv_obj_add_event_cb(day1_wind, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WIND_DAY1, weaInfo.day1_windDir);

    lv_obj_t *wea_d1_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wea_d1_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wea_d1_cont, LV_ALIGN_TOP_LEFT, 0, 210);
    lv_obj_clear_flag(wea_d1_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wea_d1_cont, LV_DIR_HOR);

    lv_obj_t *day1_wea = lv_label_create(wea_d1_cont);
    lv_obj_add_style(day1_wea, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day1_wea, 80, 30);
    lv_label_set_long_mode(day1_wea, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day1_wea, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WEA_DAY1, day1_wea, "%s");
    lv_obj_add_event_cb(day1_wea, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEA_DAY1, weaInfo.day1_Wea);

    // day 2
    lv_obj_t *data_d2_cont = lv_obj_create(scr_2);
    lv_obj_add_style(data_d2_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(data_d2_cont, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_clear_flag(data_d2_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(data_d2_cont, LV_DIR_HOR);

    lv_obj_t *date2Label = lv_label_create(data_d2_cont);
    lv_obj_add_style(date2Label, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(DATA_DAY2, date2Label, "%s");
    lv_obj_align(date2Label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(date2Label, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(DATA_DAY2, weaInfo.day2_Date);

    lv_obj_t *weather_icon_d2_cont = lv_obj_create(scr_2);
    lv_obj_add_style(weather_icon_d2_cont, &icon_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(weather_icon_d2_cont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_clear_flag(weather_icon_d2_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(weather_icon_d2_cont, LV_DIR_HOR);

    lv_obj_t *day2_weather_Img = lv_img_create(weather_icon_d2_cont);
    lv_img_set_src(day2_weather_Img, "A:99@1x.png");
    lv_obj_align(day2_weather_Img, LV_ALIGN_CENTER, 0, 0);
    lv_msg_subsribe_obj(WEATHER_IMG_DAY2, day2_weather_Img, "A:%d@1x.png");
    lv_obj_add_event_cb(day2_weather_Img, img_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEATHER_IMG_DAY2, &weaInfo.daily_weather_code[1]);

    lv_obj_t *temp_d2_cont = lv_obj_create(scr_2);
    lv_obj_add_style(temp_d2_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(temp_d2_cont, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_clear_flag(temp_d2_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(temp_d2_cont, LV_DIR_HOR);

    lv_obj_t *day2_temp = lv_label_create(temp_d2_cont);
    lv_obj_add_style(day2_temp, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(TEMP_DAY2, day2_temp, "%s");
    lv_obj_align(day2_temp, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(day2_temp, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    sprintf(buf, "%2d~%2d°", weaInfo.daily_min[1], weaInfo.daily_max[1]);
    lv_msg_send(TEMP_DAY2, buf);

    lv_obj_t *wind_d2_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wind_d2_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wind_d2_cont, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_clear_flag(wind_d2_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wind_d2_cont, LV_DIR_HOR);

    lv_obj_t *day2_wind = lv_label_create(wind_d2_cont);
    lv_obj_add_style(day2_wind, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day2_wind, 80, 30);
    lv_label_set_long_mode(day2_wind, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day2_wind, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WIND_DAY2, day2_wind, "%s");
    lv_obj_add_event_cb(day2_wind, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WIND_DAY2, weaInfo.day2_windDir);

    lv_obj_t *wea_d2_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wea_d2_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wea_d2_cont, LV_ALIGN_TOP_MID, 0, 210);
    lv_obj_clear_flag(wea_d2_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wea_d2_cont, LV_DIR_HOR);

    lv_obj_t *day2_wea = lv_label_create(wea_d2_cont);
    lv_obj_add_style(day2_wea, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day2_wea, 80, 30);
    lv_label_set_long_mode(day2_wea, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day2_wea, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WEA_DAY2, day2_wea, "%s");
    lv_obj_add_event_cb(day2_wea, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEA_DAY2, weaInfo.day2_Wea);

    // day 3
    lv_obj_t *data_d3_cont = lv_obj_create(scr_2);
    lv_obj_add_style(data_d3_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(data_d3_cont, LV_ALIGN_TOP_RIGHT, 0, 40);
    lv_obj_clear_flag(data_d3_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(data_d3_cont, LV_DIR_HOR);

    lv_obj_t *date3Label = lv_label_create(data_d3_cont);
    lv_obj_add_style(date3Label, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(DATA_DAY3, date3Label, "%s");
    lv_obj_align(date3Label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(date3Label, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(DATA_DAY3, weaInfo.day3_Date);

    lv_obj_t *weather_icon_d3_cont = lv_obj_create(scr_2);
    lv_obj_add_style(weather_icon_d3_cont, &icon_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(weather_icon_d3_cont, LV_ALIGN_TOP_RIGHT, -10, 80);
    lv_obj_clear_flag(weather_icon_d3_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(weather_icon_d3_cont, LV_DIR_HOR);

    lv_obj_t *day3_weather_Img = lv_img_create(weather_icon_d3_cont);
    lv_img_set_src(day3_weather_Img, "A:99@1x.png");
    lv_obj_align(day3_weather_Img, LV_ALIGN_CENTER, 0, 0);
    lv_msg_subsribe_obj(WEATHER_IMG_DAY3, day3_weather_Img, "A:%d@1x.png");
    lv_obj_add_event_cb(day3_weather_Img, img_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEATHER_IMG_DAY3, &weaInfo.daily_weather_code[2]);

    lv_obj_t *temp_d3_cont = lv_obj_create(scr_2);
    lv_obj_add_style(temp_d3_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(temp_d3_cont, LV_ALIGN_TOP_RIGHT, 0, 150);
    lv_obj_clear_flag(temp_d3_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(temp_d3_cont, LV_DIR_HOR);

    lv_obj_t *day3_temp = lv_label_create(temp_d3_cont);
    lv_obj_add_style(day3_temp, &chFont_style, LV_STATE_DEFAULT);
    lv_msg_subsribe_obj(TEMP_DAY3, day3_temp, "%s");
    lv_obj_align(day3_temp, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(day3_temp, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    sprintf(buf, "%2d~%2d°", weaInfo.daily_min[2], weaInfo.daily_max[2]);
    lv_msg_send(TEMP_DAY3, buf);

    lv_obj_t *wind_d3_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wind_d3_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wind_d3_cont, LV_ALIGN_TOP_RIGHT, 0, 180);
    lv_obj_clear_flag(wind_d3_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wind_d3_cont, LV_DIR_HOR);

    lv_obj_t *day3_wind = lv_label_create(wind_d3_cont);
    lv_obj_add_style(day3_wind, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day3_wind, 80, 30);
    lv_label_set_long_mode(day3_wind, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day3_wind, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WIND_DAY3, day3_wind, "%s");
    lv_obj_add_event_cb(day3_wind, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WIND_DAY3, weaInfo.day3_windDir);

    lv_obj_t *wea_d3_cont = lv_obj_create(scr_2);
    lv_obj_add_style(wea_d3_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_align(wea_d3_cont, LV_ALIGN_TOP_RIGHT, 0, 210);
    lv_obj_clear_flag(wea_d3_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(wea_d3_cont, LV_DIR_HOR);

    lv_obj_t *day3_wea = lv_label_create(wea_d3_cont);
    lv_obj_add_style(day3_wea, &chFont_style, LV_STATE_DEFAULT);
    // lv_obj_set_size(day3_wea, 80, 30);
    lv_label_set_long_mode(day3_wea, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(day3_wea, LV_ALIGN_CENTER, 0, 0);

    lv_msg_subsribe_obj(WEA_DAY3, day3_wea, "%s");
    lv_obj_add_event_cb(day3_wea, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(WEA_DAY3, weaInfo.day3_Wea);



    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(scr_2, anim_type, 500, 200, false);
    }
    else
    {
        lv_scr_load(scr_2);
    }
    update_weather_info(weaInfo, weaInfo.update_3day_weather_flag);
}


void display_weather_init(struct Weather weaInfo, lv_scr_load_anim_t anim_type)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == scr_1)
        return;

    weather_gui_release();
    lv_obj_clean(act_obj); // 清空此前页面

    scr_1 = lv_obj_create(NULL);
    lv_obj_add_style(scr_1, &default_style, LV_STATE_DEFAULT);
    
    // weatherImg = lv_img_create(scr_1);
    // lv_img_set_src(weatherImg, weaImage_map[0]);

    lv_obj_t *weatherImg = lv_img_create(scr_1);
    // lv_img_set_src(weatherImg, "S:/weather/icon/x2/99@2x.png");
    lv_img_set_src(weatherImg, "A:99@2x.png");

    lv_obj_t *city_label_cont = lv_obj_create(scr_1);
    lv_obj_add_style(city_label_cont, &label_cont_style, LV_STATE_DEFAULT);
    lv_obj_set_width(city_label_cont, 75);
    lv_obj_align(city_label_cont, LV_ALIGN_TOP_LEFT, 0, 10);
    lv_obj_clear_flag(city_label_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(city_label_cont, LV_DIR_HOR);

    lv_obj_t *cityLabel = lv_label_create(city_label_cont);
    lv_obj_add_style(cityLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(cityLabel, true);
    // lv_label_set_text(cityLabel, "上海");
    lv_obj_set_style_text_font(cityLabel, &weiruanyahei_27, 0);

    lv_obj_t *btn = lv_btn_create(scr_1);
    lv_obj_add_style(btn, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_pos(btn, 75, 15);
    lv_obj_set_size(btn, 50, 25);
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_ORANGE), LV_STATE_DEFAULT);

    lv_obj_t *btnLabel = lv_label_create(btn);
    lv_obj_add_style(btnLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_obj_align(btnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(btnLabel, airQualityCh[0]);

    lv_obj_t *txtLabel = lv_label_create(scr_1);
    lv_obj_add_style(txtLabel, &chFont_style, LV_STATE_DEFAULT);
    // lvgl8之前版本，模式一旦设置 LV_LABEL_LONG_SCROLL_CIRCULAR
    // 宽度恒定等于当前文本的长度，所以下面先设置以下长度
    // lv_label_set_text(txtLabel, "最低气温12°C, ");
    lv_obj_set_size(txtLabel, 120, 30);
    lv_label_set_long_mode(txtLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // lv_label_set_text_fmt(txtLabel, "最低气温%d°C, 最高气温%d°C, %s%d 级.   ", 15, 20, "西北风", 0);

    lv_obj_t *clockLabel_1 = lv_label_create(scr_1);
    lv_obj_add_style(clockLabel_1, &numberBig_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_1, true);
    // lv_label_set_text_fmt(clockLabel_1, "%02d#ffa500 %02d", 10, 52);
    lv_label_set_text(clockLabel_1, "88#ffa500 88");

    lv_obj_t *clockLabel_2 = lv_label_create(scr_1);
    lv_obj_add_style(clockLabel_2, &numberSmall_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_2, true);
    lv_label_set_text_fmt(clockLabel_2, "%02d", 00);

    lv_obj_t *dateLabel = lv_label_create(scr_1);
    lv_obj_add_style(dateLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(dateLabel, "%2d月%2d日   周%s", 11, 23, weekDayCh[1]);

    lv_obj_t *tempImg = lv_img_create(scr_1);
    lv_img_set_src(tempImg, &temp);
    lv_img_set_zoom(tempImg, 180);
    lv_obj_t *tempBar = lv_bar_create(scr_1);
    lv_obj_add_style(tempBar, &bar_style, LV_STATE_DEFAULT);
    lv_bar_set_range(tempBar, -20, 60); // 设置进度条表示的温度为-20~60
    lv_obj_set_size(tempBar, 60, 12);
    lv_obj_set_style_bg_color(tempBar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    lv_bar_set_value(tempBar, 10, LV_ANIM_ON);
    lv_obj_t *tempLabel = lv_label_create(scr_1);
    lv_obj_add_style(tempLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(tempLabel, "%2d°C", weaInfo.indoor_temperature);

    lv_obj_t *humiImg = lv_img_create(scr_1);
    lv_img_set_src(humiImg, &humi);
    lv_img_set_zoom(humiImg, 180);
    lv_obj_t *humiBar = lv_bar_create(scr_1);
    lv_obj_add_style(humiBar, &bar_style, LV_STATE_DEFAULT);
    lv_bar_set_range(humiBar, 0, 100);
    lv_obj_set_size(humiBar, 60, 12);
    lv_obj_set_style_bg_color(humiBar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_bar_set_value(humiBar, 49, LV_ANIM_ON);
    lv_obj_t *humiLabel = lv_label_create(scr_1);
    lv_obj_add_style(humiLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(humiLabel, "%2d%%", weaInfo.indoor_huimdity);

    // 太空人图标
    spaceImg = lv_img_create(scr_1);
    lv_img_set_src(spaceImg, manImage_map[0]);
    // lv_obj_t *spaceImg = lv_gif_create(scr_1);
    // lv_gif_set_src(spaceImg, "A:space8080.gif");

    // 绘制图形
    lv_obj_align(weatherImg, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_align(cityLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(txtLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_obj_align(tempImg, LV_ALIGN_LEFT_MID, 10, 70);
    lv_obj_align(tempBar, LV_ALIGN_LEFT_MID, 35, 70);
    lv_obj_align(tempLabel, LV_ALIGN_LEFT_MID, 103, 70);
    lv_obj_align(humiImg, LV_ALIGN_LEFT_MID, 0, 100);
    lv_obj_align(humiBar, LV_ALIGN_LEFT_MID, 35, 100);
    lv_obj_align(humiLabel, LV_ALIGN_LEFT_MID, 103, 100);
    lv_obj_align(spaceImg, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    lv_obj_align(clockLabel_1, LV_ALIGN_LEFT_MID, 0, 10);
    lv_obj_align(clockLabel_2, LV_ALIGN_LEFT_MID, 165, 9);
    lv_obj_align(dateLabel, LV_ALIGN_LEFT_MID, 10, 32);

    lv_msg_subsribe_obj(CITY_NAME, cityLabel, "%s");
    lv_obj_add_event_cb(cityLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_send(CITY_NAME, weaInfo.cityname);

    lv_msg_subsribe_obj(AIR_QUALITY, btnLabel, "%s");
    lv_obj_add_event_cb(btnLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_msg_subsribe_obj(TXTLABEL, txtLabel, "%s");
    lv_obj_add_event_cb(txtLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_send(TXTLABEL, "正在获取天气数据");

    lv_msg_subsribe_obj(WEATHER_IMG, weatherImg, "A:%d@2x.png");
    lv_obj_add_event_cb(weatherImg, img_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_send(WEATHER_IMG, &weaInfo.weather_code);

    lv_msg_subsribe_obj(WEATHER_MAIN_TIME, clockLabel_1, "%s");
    lv_obj_add_event_cb(clockLabel_1, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_send(WEATHER_MAIN_TIME, "00#ffa500 00");

    lv_msg_subsribe_obj(DATA_LABEL, dateLabel, "%s");
    lv_obj_add_event_cb(dateLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_msg_subsribe_obj(WEATHER_MAIN_TIME_SEC, clockLabel_2, "%02d");
    lv_obj_add_event_cb(clockLabel_2, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_msg_subsribe_obj(WEATHER_MAIN_TEMP, tempLabel, "%d°C");
    lv_obj_add_event_cb(tempLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(WEATHER_MAIN_TEMP, tempBar, NULL);
    lv_obj_add_event_cb(tempBar, bar_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_msg_subsribe_obj(WEATHER_MAIN_HUMI, humiLabel, "%d%%");
    lv_obj_add_event_cb(humiLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(WEATHER_MAIN_HUMI, humiBar, NULL);
    lv_obj_add_event_cb(humiBar, bar_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_msg_send(WEATHER_MAIN_TEMP, &(weaInfo.indoor_temperature));
    lv_msg_send(WEATHER_MAIN_HUMI, &(weaInfo.indoor_huimdity));



    if (LV_SCR_LOAD_ANIM_NONE != anim_type)
    {
        lv_scr_load_anim(scr_1, anim_type, 500, 300, false);
    }
    else
    {
        lv_scr_load(scr_1);
    }
    update_weather_info(weaInfo, weaInfo.update_weather_flag);
}


void update_indoor_temp_humi(int temp, int humi)
{
    lv_msg_send(WEATHER_MAIN_TEMP, &temp);
    lv_msg_send(WEATHER_MAIN_HUMI, &humi);
}

void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type)
{
    char buf[30] = "";
    if (scr_1 != NULL)
    {
        sprintf(buf, "%02d#ffa500 %02d", timeInfo.hour, timeInfo.minute);
        lv_msg_send(WEATHER_MAIN_TIME, buf);
        lv_msg_send(WEATHER_MAIN_TIME_SEC, &timeInfo.second);
        sprintf(buf, "%2d月%2d日   周%s", timeInfo.month, timeInfo.day, weekDayCh[timeInfo.weekday]);
        lv_msg_send(DATA_LABEL, buf);
    }
    if (scr_2 != NULL)
    {
        sprintf(buf, "%2d月%2d日  周%s  %02d:%02d", timeInfo.month, timeInfo.day, weekDayCh[timeInfo.weekday], timeInfo.hour, timeInfo.minute);
        lv_msg_send(DATA, buf);
    }
}

void weather_gui_release()
{
    if (scr_1 != NULL)
    {
        lv_obj_clean(scr_1);
        // lv_obj_del(scr_1);
        scr_1 = NULL;
        // lv_obj_del(spaceImg);
        spaceImg = NULL;
    }

    if (scr_2 != NULL)
    {
        lv_obj_clean(scr_2);
        // lv_obj_del(scr_2);
        scr_2 = NULL;
    }
}

void weather_gui_del(void)
{
    weather_gui_release();

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&chFont_style);
    // lv_style_reset(&numberSmall_style);
    // lv_style_reset(&numberBig_style);
    // lv_style_reset(&btn_style);
    // lv_style_reset(&bar_style);
}

void display_space(void)
{
    static int _spaceIndex = 0;
    if (NULL != scr_1 && lv_scr_act() == scr_1)
    {
        lv_img_set_src(spaceImg, manImage_map[_spaceIndex]);
        _spaceIndex = (_spaceIndex + 1) % 10;
    }
}

int airQulityLevel(int q)
{
    if (q < 50)
    {
        return 0;
    }
    else if (q < 100)
    {
        return 1;
    }
    else if (q < 150)
    {
        return 2;
    }
    else if (q < 200)
    {
        return 3;
    }
    else if (q < 300)
    {
        return 4;
    }
    return 5;
}

static void label_event_cb(lv_event_t *e)
{
    lv_obj_t *label = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    const char *fmt = lv_msg_get_user_data(m);

    if (strstr(fmt, "%s") != NULL)
    {
        /* 字符串填入 */
        const char *v = lv_msg_get_payload(m);
        lv_label_set_text_fmt(label, fmt, v);
    }
    else
    {
        /* 数字填入 */
        const char *v = lv_msg_get_payload(m);
        lv_label_set_text_fmt(label, fmt, *v);
    }
}

static void bar_event_cb(lv_event_t *e)
{
    lv_obj_t *bar = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    const char *v = lv_msg_get_payload(m);

    lv_bar_set_value(bar, *v, LV_ANIM_OFF);
}

static void img_event_cb(lv_event_t *e)
{
    lv_obj_t *img = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    const char *fmt = lv_msg_get_user_data(m);
    const char *v = lv_msg_get_payload(m);
    char path_data[40];

    sprintf(path_data, fmt, *v);
    lv_img_set_src(img, path_data);
}