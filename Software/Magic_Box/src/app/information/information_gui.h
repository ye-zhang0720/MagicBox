#ifndef APP_INFORMATION_GUI_H
#define APP_INFORMATION_GUI_H

#include "common.h"


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void information_gui_init(struct System_information *data);
    void information_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_information;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif