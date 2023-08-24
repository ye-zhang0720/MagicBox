#include "information_gui.h"

LV_FONT_DECLARE(weiruanyahei_27);
#include "lvgl.h"

static lv_style_t default_style;
static lv_style_t label_style;
static lv_obj_t *main_scr = NULL;

void information_gui_init(struct System_information *data)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页

    lv_obj_clean(act_obj); // 清空此前页面

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &weiruanyahei_27);

    main_scr = lv_obj_create(NULL);
    lv_obj_set_size(main_scr, 240, 240);
    lv_obj_set_pos(main_scr, 0, 0);
    lv_obj_add_style(main_scr, &default_style, LV_STATE_DEFAULT);


    lv_obj_t *name_lab = lv_label_create(main_scr);
    lv_obj_align(name_lab, LV_ALIGN_TOP_LEFT, 5, 10);
    lv_obj_add_style(name_lab, &label_style, LV_PART_MAIN);
    lv_label_set_text(name_lab, "本机名称");

    lv_obj_t *frame_lab = lv_label_create(main_scr);
    lv_obj_align(frame_lab, LV_ALIGN_TOP_LEFT, 5, 50);
    lv_obj_add_style(frame_lab, &label_style, LV_PART_MAIN);
    lv_label_set_text(frame_lab, "固件版本");

    lv_obj_t *head_lab = lv_label_create(main_scr);
    lv_obj_align(head_lab, LV_ALIGN_TOP_LEFT, 5, 90);
    lv_obj_add_style(head_lab, &label_style, LV_PART_MAIN);
    lv_label_set_text(head_lab, "硬件版本");

    lv_obj_t *developer_lab = lv_label_create(main_scr);
    lv_obj_align(developer_lab, LV_ALIGN_TOP_LEFT, 5, 130);
    lv_obj_add_style(developer_lab, &label_style, LV_PART_MAIN);
    lv_label_set_text(developer_lab, "开 发 者");

    lv_obj_t *tf_lab = lv_label_create(main_scr);
    lv_obj_align(tf_lab, LV_ALIGN_TOP_LEFT, 5, 170);
    lv_obj_add_style(tf_lab, &label_style, LV_PART_MAIN);
    lv_label_set_text(tf_lab, "存储空间");

    lv_obj_t *name_lab2 = lv_label_create(main_scr);
    lv_obj_align(name_lab2, LV_ALIGN_TOP_MID, 60, 10);
    lv_obj_add_style(name_lab2, &label_style, LV_PART_MAIN);
    lv_label_set_text(name_lab2, "Magic Box");

    lv_obj_t *frame_lab2 = lv_label_create(main_scr);
    lv_obj_align(frame_lab2, LV_ALIGN_TOP_MID, 60, 50);
    lv_obj_add_style(frame_lab2, &label_style, LV_PART_MAIN);
    lv_label_set_text(frame_lab2, data->FrameVersion);

    lv_obj_t *head_lab2 = lv_label_create(main_scr);
    lv_obj_align(head_lab2, LV_ALIGN_TOP_MID, 60, 90);
    lv_obj_add_style(head_lab2, &label_style, LV_PART_MAIN);
    lv_label_set_text(head_lab2, data->HeadwareVersion);

    lv_obj_t *developer_lab2 = lv_label_create(main_scr);
    lv_obj_align(developer_lab2, LV_ALIGN_TOP_MID, 60, 130);
    lv_obj_add_style(developer_lab2, &label_style, LV_PART_MAIN);
    lv_label_set_text(developer_lab2, "西北偏北");

    lv_obj_t *tf_lab2 = lv_label_create(main_scr);
    lv_obj_align(tf_lab2, LV_ALIGN_TOP_MID, 60, 170);
    lv_obj_add_style(tf_lab2, &label_style, LV_PART_MAIN);
    lv_label_set_text(tf_lab2, data->TFFreeSpace);

    lv_obj_t *bottom_label = lv_label_create(main_scr);
    lv_obj_align(bottom_label, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_add_style(bottom_label, &label_style, LV_PART_MAIN);
    lv_label_set_text(bottom_label, "@西北偏北");

    lv_scr_load_anim(main_scr, LV_SCR_LOAD_ANIM_FADE_IN, 300, 300, false);
    // lv_scr_load(main_scr);
}

/*
 * 其他函数请根据需要添加
 */

void display_information(const char *file_name, lv_scr_load_anim_t anim_type)
{
}

void information_gui_del(void)
{
    if (NULL != main_scr)
    {
        lv_obj_clean(main_scr);
        main_scr = NULL;
    }
    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}