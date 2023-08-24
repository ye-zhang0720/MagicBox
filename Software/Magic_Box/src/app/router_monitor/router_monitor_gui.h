#ifndef APP_ROUTER_GUI_H
#define APP_ROUTER_GUI_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    void router_gui_init(void);
    void display_routor(void);
    void routor_gui_del(void);
    void setIP_address(const char *ipAddr);
    void updateData(double up_speed, double down_speed, double cpu_usage, double mem_usage, double temp_value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_router;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif