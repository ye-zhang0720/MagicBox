#include "ota_gui.h"
#include "lvgl.h"

static lv_obj_t *main_scr = NULL;

static lv_obj_t *local_ip_label;
static lv_obj_t *ap_ip_label;
static lv_obj_t *domain_label;
static lv_obj_t *title_label;

static lv_style_t default_style;
static lv_style_t label_style;
static lv_style_t bar_style;


LV_FONT_DECLARE(weiruanyahei_27);

void ota_gui_init(void)
{
    main_scr = NULL;
    local_ip_label = NULL;
    ap_ip_label = NULL;
    domain_label = NULL;
    title_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));
    
    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &weiruanyahei_27);


    lv_style_init(&bar_style);
    lv_style_set_bg_color(&bar_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&bar_style, 2);
    lv_style_set_border_color(&bar_style, lv_color_hex(0xFFFFFF));
    lv_style_set_pad_top(&bar_style, 1); // 指示器到背景四周的距离
    lv_style_set_pad_bottom(&bar_style, 1);
    lv_style_set_pad_left(&bar_style, 1);
    lv_style_set_pad_right(&bar_style, 1);
}

void display_ota_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == main_scr)
        return;

    if (NULL != main_scr)
    {
        lv_obj_clean(main_scr);
    }

    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    main_scr = lv_obj_create(NULL);
    lv_obj_set_size(main_scr, 240 , 240);
    lv_obj_set_pos(main_scr, 0, 0);
    lv_obj_add_style(main_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(main_scr);
    lv_obj_add_style(title_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    domain_label = lv_label_create(main_scr);
    lv_obj_add_style(domain_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(domain_label, LV_ALIGN_BOTTOM_LEFT, 5, -90);

    ap_ip_label = lv_label_create(main_scr);
    lv_obj_add_style(ap_ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(ap_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -50);

    local_ip_label = lv_label_create(main_scr);
    lv_obj_add_style(local_ip_label, &label_style, LV_STATE_DEFAULT);
    lv_obj_align(local_ip_label, LV_ALIGN_BOTTOM_LEFT, 5, -20);

    lv_obj_t *barLabel = lv_label_create(main_scr);
    lv_obj_add_style(barLabel, &label_style, LV_STATE_DEFAULT);
    lv_label_set_text_fmt(barLabel, "%2f%%", 0);


    lv_obj_t *Bar = lv_bar_create(main_scr);
    lv_obj_add_style(Bar, &bar_style, LV_STATE_DEFAULT);
    lv_bar_set_range(Bar, 0, 100);
    lv_obj_set_size(Bar, 60, 12);
    lv_obj_set_style_bg_color(Bar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    lv_bar_set_value(Bar, 0, LV_ANIM_ON);
    lv_obj_align(Bar, LV_ALIGN_LEFT_MID, 35, 70);

    lv_msg_subsribe_obj(Bar, barLabel, "%d°C");
    lv_obj_add_event_cb(barLabel, label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(Bar, Bar, NULL);
    lv_obj_add_event_cb(Bar, bar_event_cb, LV_EVENT_MSG_RECEIVED, NULL);

    lv_scr_load(main_scr);
}

void display_ota(const char *title, const char *domain,
                     const char *info, const char *ap_ip,
                     lv_scr_load_anim_t anim_type)
{
    display_ota_init();

    lv_label_set_text(title_label, title);

    lv_label_set_text(domain_label, domain);

    lv_label_set_text(ap_ip_label, ap_ip);

    lv_label_set_text(local_ip_label, info);
}

void ota_gui_del(void)
{
    if (NULL != main_scr)
    {
        lv_obj_clean(main_scr); // 清空此前页面
        main_scr = NULL;
        local_ip_label = NULL;
        ap_ip_label = NULL;
        domain_label = NULL;
        title_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&label_style);
}