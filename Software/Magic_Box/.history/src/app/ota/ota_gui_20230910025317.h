#ifndef APP_OTA_GUI_H
#define APP_OTA_GUI_H

// #include "lv_port_indev.h"

enum
{
    ProgressBar,
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void ota_gui_init(void);
    void display_ota_init(void);
    void update_progress(int a);
    void display_ota(const char *title, const char *domain,
                        const char *info, const char *ap_ip,
                         lv_scr_load_anim_t anim_type);
    void ota_gui_del(void);
    static void show_progress();

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_ota;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif