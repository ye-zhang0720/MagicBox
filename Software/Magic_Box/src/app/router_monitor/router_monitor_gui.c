#include "router_monitor_gui.h"
#include "lvgl.h"
LV_FONT_DECLARE(weiruanyahei_27);

static lv_style_t default_style;
static lv_style_t label_style;

static lv_obj_t *monitor_page = NULL;

// static lv_obj_t* label1;
static lv_obj_t *upload_label;
static lv_obj_t *down_label;
static lv_obj_t *up_speed_label;
static lv_obj_t *up_speed_unit_label;
static lv_obj_t *down_speed_label;
static lv_obj_t *down_speed_unit_label;
static lv_obj_t *cpu_bar;
static lv_obj_t *cpu_value_label;
static lv_obj_t *mem_bar;
static lv_obj_t *mem_value_label;
static lv_obj_t *temp_value_label;
static lv_obj_t *temperature_arc;
static lv_obj_t *ip_label;
static lv_obj_t *chart;

static lv_chart_series_t *ser1;
static lv_chart_series_t *ser2;

static lv_style_t arc_indic_style;
static lv_style_t font_22;

lv_coord_t up_speed_max = 0;
lv_coord_t down_speed_max = 0;

lv_coord_t upload_serise[20] = {0};
lv_coord_t download_serise[20] = {0};

void updateNetworkInfoLabel(double up_speed, double down_speed)
{
    if (up_speed < 100.0)
    {
        // < 99.99 K/S
        lv_label_set_text_fmt(up_speed_label, "%.2f", up_speed);
        lv_label_set_text(up_speed_unit_label, "K/s");
    }
    else if (up_speed < 1000.0)
    {
        // 999.9 K/S
        lv_label_set_text_fmt(up_speed_label, "%.1f", up_speed);
        lv_label_set_text(up_speed_unit_label, "K/s");
    }
    else if (up_speed < 100000.0)
    {
        // 99.99 M/S
        up_speed /= 1024.0;
        lv_label_set_text_fmt(up_speed_label, "%.2f", up_speed);
        lv_label_set_text(up_speed_unit_label, "M/s");
    }
    else if (up_speed < 1000000.0)
    {
        // 999.9 M/S
        up_speed = up_speed / 1024.0;
        lv_label_set_text_fmt(up_speed_label, "%.1f", up_speed);
        lv_label_set_text(up_speed_unit_label, "M/s");
    }

    if (down_speed < 100.0)
    {
        // < 99.99 K/S
        lv_label_set_text_fmt(down_speed_label, "%.2f", down_speed);
        lv_label_set_text(down_speed_unit_label, "K/s");
    }
    else if (down_speed < 1000.0)
    {
        // 999.9 K/S
        lv_label_set_text_fmt(down_speed_label, "%.1f", down_speed);
        lv_label_set_text(down_speed_unit_label, "K/s");
    }
    else if (down_speed < 100000.0)
    {
        // 99.99 M/S
        down_speed /= 1024.0;
        lv_label_set_text_fmt(down_speed_label, "%.2f", down_speed);
        lv_label_set_text(down_speed_unit_label, "M/s");
    }
    else if (down_speed < 1000000.0)
    {
        // 999.9 M/S
        down_speed = down_speed / 1024.0;
        lv_label_set_text_fmt(down_speed_label, "%.1f", down_speed);
        lv_label_set_text(down_speed_unit_label, "M/s");
    }
    lv_obj_align_to(up_speed_label, up_speed_unit_label, LV_ALIGN_OUT_LEFT_BOTTOM, -3, 0);
    lv_obj_align_to(down_speed_label, down_speed_unit_label, LV_ALIGN_OUT_LEFT_BOTTOM, -3, 0);
}

lv_coord_t max(lv_coord_t a, lv_coord_t b)
{
    if (a > b)
        return a;
    else
        return b;
}

void updateCPU(double cpu_usage)
{
    lv_bar_set_value(cpu_bar, cpu_usage, LV_ANIM_OFF);
    lv_label_set_text_fmt(cpu_value_label, "%2.1f%%", cpu_usage);
}

void updateMem(double mem_usage)
{
    lv_bar_set_value(mem_bar, mem_usage, LV_ANIM_OFF);
    lv_label_set_text_fmt(mem_value_label, "%2.0f%%", mem_usage);
}

void updateTempareture(double temp_value)
{
    lv_label_set_text_fmt(temp_value_label, "%2.0f°C", temp_value);
    lv_arc_set_value(temperature_arc, temp_value); // 设置现在的值
    // uint16_t end_value = 120 + 300 * temp_value / 100.0f;
    // lv_color_t arc_color = temp_value > 75 ? lv_color_hex(0xff5d18) : lv_color_hex(0x50ff7d);
    // lv_style_set_line_color(&arc_indic_style, arc_color);
    // lv_obj_add_style(temperature_arc, &arc_indic_style, LV_PART_INDICATOR);
    // lv_arc_set_end_angle(temperature_arc, end_value);
}

lv_coord_t updateNetSeries(lv_coord_t *series, double speed)
{
    lv_coord_t local_max = series[0];
    for (int index = 0; index < 19; index++)
    {
        series[index] = series[index + 1];
        if (local_max < series[index])
        {
            local_max = series[index];
        }
    }
    series[19] = (lv_coord_t)speed;
    if (local_max < series[19])
        local_max = series[19];

    // Serial.print(speed);
    // Serial.print("->");
    // Serial.print(series[9]);
    // Serial.print("    |");
    // for (int i = 0; i < 10; i++)
    // {
    //     Serial.print(series[i]);
    //     Serial.print(" ");
    // }
    // Serial.println();

    return local_max;
}

void updateChartRange()
{
    lv_coord_t max_speed = max(down_speed_max, up_speed_max);
    max_speed = max(max_speed, (lv_coord_t)16);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, (lv_coord_t)(max_speed * 1.1));
}

void updateData(double up_speed, double down_speed, double cpu_usage, double mem_usage, double temp_value)
{
    updateNetworkInfoLabel(up_speed, down_speed);
    down_speed_max = updateNetSeries(download_serise, down_speed);
    up_speed_max = updateNetSeries(upload_serise, up_speed);
    lv_chart_set_ext_y_array(chart, ser2, download_serise);
    lv_chart_set_ext_y_array(chart, ser1, upload_serise);
    lv_chart_refresh(chart); /*Required after direct set*/
    updateChartRange();
    updateCPU(cpu_usage);
    updateMem(mem_usage);
    updateTempareture(temp_value);
}

void router_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x7381a2));
    lv_style_set_bg_opa(&default_style, LV_OPA_100);

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_16);

    lv_style_init(&font_22);
    lv_style_set_text_font(&font_22, &lv_font_montserrat_22);

    // lv_style_set_text_font(&iconfont, &lv_font_montserrat_16); // 重置样式

    display_routor();
}

/*
 * 刷新IP地址
 */
void setIP_address(const char *ipAddr)
{
    lv_label_set_text(ip_label, ipAddr);
}

/*
 * 显示
 */

void display_routor()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == monitor_page)
        return;

    if (NULL != monitor_page)
    {
        lv_obj_clean(monitor_page);
    }

    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    monitor_page = lv_obj_create(NULL);
    lv_obj_set_size(monitor_page, 240, 240);
    lv_obj_set_pos(monitor_page, 0, 0);
    lv_obj_add_style(monitor_page, &default_style, LV_STATE_DEFAULT);

    ip_label = lv_label_create(monitor_page);
    // lv_label_set_text(ip_label, WiFi.localIP().toString().c_str());
    lv_label_set_text(ip_label, "Connecting...");
    lv_obj_add_style(ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_set_pos(ip_label, 10, 211);
    lv_obj_align(ip_label, LV_ALIGN_BOTTOM_LEFT, 5, 0);

    // setIP_address("192.168.100.199");

    lv_obj_t *cont = lv_obj_create(monitor_page);
    lv_obj_set_width(cont, 230);
    lv_obj_set_height(cont, 120);
    // lv_obj_set_pos(cont, 5, 5);
    // lv_obj_set_style_border_color(cont, lv_color_hex(0x081418), 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x081418), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // Upload & Download Symbol
    upload_label = lv_label_create(cont);
    lv_obj_set_style_text_color(upload_label, lv_palette_main(LV_PALETTE_RED), LV_STATE_DEFAULT);
    lv_label_set_text(upload_label, LV_SYMBOL_UPLOAD);
    lv_obj_align(upload_label, LV_ALIGN_TOP_LEFT, -5, -5);

    down_label = lv_label_create(cont);
    lv_obj_set_style_text_color(down_label, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_DEFAULT);
    lv_label_set_text(down_label, LV_SYMBOL_DOWNLOAD);
    lv_obj_align(down_label, LV_ALIGN_TOP_MID, 8, -5);

    up_speed_unit_label = lv_label_create(cont);
    lv_label_set_text(up_speed_unit_label, "K/S");
    lv_obj_set_style_text_color(up_speed_unit_label, lv_color_hex(0x838a99), LV_STATE_DEFAULT);
    // lv_obj_add_style(up_speed_unit_label, &font_22, 0);
    // lv_obj_align(up_speed_unit_label, LV_ALIGN_TOP_MID, -18, 0);
    lv_obj_align(up_speed_unit_label, LV_ALIGN_TOP_MID, -16, 0);

    // Upload & Download Speed Display
    up_speed_label = lv_label_create(cont);
    lv_label_set_text(up_speed_label, "0");
    lv_obj_set_style_text_color(up_speed_label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_add_style(up_speed_label, &font_22, 0);
    lv_obj_align_to(up_speed_label, up_speed_unit_label, LV_ALIGN_OUT_LEFT_BOTTOM, -3, 0);
 

    down_speed_unit_label = lv_label_create(cont);
    lv_label_set_text(down_speed_unit_label, "M/S");
    lv_obj_set_style_text_color(down_speed_unit_label, lv_color_hex(0x838a99), LV_STATE_DEFAULT);
    // lv_obj_add_style(down_speed_unit_label, &font_22, 0);
    // lv_obj_align(down_speed_unit_label, LV_ALIGN_TOP_RIGHT, 10, 0);
    lv_obj_align(down_speed_unit_label, LV_ALIGN_TOP_RIGHT, 3, 0);

    down_speed_label = lv_label_create(cont);
    lv_label_set_text(down_speed_label, "0");
    lv_obj_set_style_text_color(down_speed_label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_add_style(down_speed_label, &font_22, 0);
    // lv_obj_align_to(down_speed_label, down_speed_unit_label, LV_ALIGN_OUT_LEFT_MID, -3, 0);
    lv_obj_align_to(down_speed_label, down_speed_unit_label, LV_ALIGN_OUT_LEFT_BOTTOM, 6, 0);

    // 绘制曲线图
    /*Create a chart*/
    chart = lv_chart_create(cont);
    lv_obj_set_size(chart, 220, 70);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, 2);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 4096);
    lv_chart_set_point_count(chart, 20); // 设置显示点数
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

    /*Add a faded are effect*/
    lv_obj_set_style_bg_opa(chart, LV_OPA_50, 0);

    lv_obj_set_style_bg_grad_dir(chart, LV_GRAD_DIR_VER, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(chart, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(chart, 255, LV_STATE_DEFAULT);

    /*Add two data series*/
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);

    // /*Directly set points on 'ser2'*/
    lv_chart_set_ext_y_array(chart, ser2, download_serise);
    lv_chart_set_ext_y_array(chart, ser1, upload_serise);

    lv_chart_refresh(chart); /*Required after direct set*/

    // 绘制进度条  CPU 占用
    lv_obj_t *cpu_title = lv_label_create(monitor_page);
    lv_label_set_text(cpu_title, "CPU");
    lv_obj_set_style_text_color(cpu_title, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_pos(cpu_title, 5, 140);

    cpu_value_label = lv_label_create(monitor_page);
    lv_label_set_text(cpu_value_label, "0%");
    lv_obj_add_style(cpu_value_label, &font_22, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cpu_value_label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_pos(cpu_value_label, 85, 135);

    cpu_bar = lv_bar_create(monitor_page);
    lv_obj_set_size(cpu_bar, 130, 10);
    lv_obj_set_pos(cpu_bar, 5, 160);

    lv_color_t bar_indic_color = lv_color_hex(0x63d0fc);
    lv_color_t bar_bg_color = lv_color_hex(0x1e3644);
    lv_color_t cont_color = lv_color_hex(0x081418);
    lv_obj_set_style_bg_color(cpu_bar, bar_bg_color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cpu_bar, bar_indic_color, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(cpu_bar, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cpu_bar, 2, LV_PART_INDICATOR);

    lv_obj_set_style_border_color(cpu_bar, cont_color, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cpu_bar, cont_color, LV_PART_INDICATOR);

    lv_obj_set_style_border_side(cpu_bar, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM, LV_PART_INDICATOR);
    lv_obj_set_style_radius(cpu_bar, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cpu_bar, 0, LV_PART_INDICATOR);

    // 绘制内存占用
    lv_obj_t *men_title = lv_label_create(monitor_page);
    lv_label_set_text(men_title, "Memory");
    lv_obj_set_style_text_color(men_title, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_pos(men_title, 5, 180);

    mem_value_label = lv_label_create(monitor_page);
    lv_label_set_text(mem_value_label, "0%");
    lv_obj_add_style(mem_value_label, &font_22, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(mem_value_label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_pos(mem_value_label, 85, 175);

    mem_bar = lv_bar_create(monitor_page);
    lv_obj_set_size(mem_bar, 130, 10);
    lv_obj_set_pos(mem_bar, 5, 200);

    lv_obj_set_style_bg_color(mem_bar, bar_bg_color, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mem_bar, bar_indic_color, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(mem_bar, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(mem_bar, 2, LV_PART_INDICATOR);

    lv_obj_set_style_border_color(mem_bar, cont_color, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(mem_bar, cont_color, LV_PART_INDICATOR);

    lv_obj_set_style_border_side(mem_bar, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM, LV_PART_INDICATOR);
    lv_obj_set_style_radius(mem_bar, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(mem_bar, 0, LV_PART_INDICATOR);

    // 绘制温度表盘
    static lv_style_t arc_style;
    lv_style_reset(&arc_style);
    lv_style_init(&arc_style);
    lv_style_set_bg_opa(&arc_style, LV_OPA_TRANSP);
    lv_style_set_border_opa(&arc_style, LV_OPA_TRANSP);
    lv_style_set_line_width(&arc_style, 100);
    lv_style_set_line_color(&arc_style, lv_color_hex(0x081418));
    lv_style_set_line_rounded(&arc_style, false);

    lv_style_init(&arc_indic_style);
    lv_style_set_line_width(&arc_indic_style, 5);
    // lv_style_set_pad_left(&arc_indic_style,  5);
    // lv_style_set_line_color(&arc_indic_style, LV_STATE_DEFAULT, lv_color_hex(0x50ff7d));
    lv_style_set_line_color(&arc_indic_style, lv_color_hex(0xff5d18));
    // lv_style_set_line_rounded(&arc_indic_style, LV_STATE_DEFAULT, false);

    temperature_arc = lv_arc_create(monitor_page);
    lv_obj_remove_style(temperature_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(temperature_arc, LV_OBJ_FLAG_CHECKABLE);
    // lv_arc_set_bg_angles(temperature_arc, 0, 300);
    // lv_arc_set_rotation(temperature_arc, 135);//设置值的范围
    lv_obj_set_size(temperature_arc, 90, 90);
    lv_obj_add_style(temperature_arc, &arc_style, LV_STATE_DEFAULT);
    lv_obj_add_style(temperature_arc, &arc_indic_style, LV_PART_INDICATOR);
    lv_obj_align(temperature_arc, LV_ALIGN_BOTTOM_RIGHT, -8, -10);
    lv_obj_set_style_arc_color(temperature_arc, lv_color_hex(0xff0000), LV_PART_INDICATOR);

    lv_arc_set_value(temperature_arc, 30); // 设置现在的值

    temp_value_label = lv_label_create(monitor_page);
    lv_label_set_text(temp_value_label, "30°C");
    lv_obj_set_style_text_font(temp_value_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(temp_value_label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_align_to(temp_value_label, temperature_arc, LV_ALIGN_CENTER, 0, 0);

    lv_scr_load(monitor_page);
}

void routor_gui_del(void)
{
    if (NULL != monitor_page)
    {
        lv_obj_clean(monitor_page);

        monitor_page = NULL;
        upload_label = NULL;
        down_label = NULL;
        up_speed_label = NULL;
        up_speed_unit_label = NULL;
        down_speed_label = NULL;
        down_speed_unit_label = NULL;
        cpu_bar = NULL;
        cpu_value_label = NULL;
        mem_bar = NULL;
        mem_value_label = NULL;
        temp_value_label = NULL;
        temperature_arc = NULL;
        ip_label = NULL;
        chart = NULL;

        ser1 = NULL;
        ser2 = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}