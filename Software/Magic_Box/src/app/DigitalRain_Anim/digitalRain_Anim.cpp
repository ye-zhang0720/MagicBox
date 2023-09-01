#include "digitalRain_Anim.h"
#include "sys/app_controller.h"
#include "common.h"
#include "network.h"
#include <stdint.h>
#include <TFT_eSPI.h>
#include <DigitalRainAnimation.hpp>

LV_IMG_DECLARE(app_DigitalRain);

#define DIGITAL_RAIN_APP_NAME "文字雨"

DigitalRainAnimation<TFT_eSPI> *matrix_effect;

static int DigitalRain_init(AppController *sys)
{
    setCpuFrequencyMhz(240);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    matrix_effect = new DigitalRainAnimation<TFT_eSPI>();
    matrix_effect->init(tft);
    matrix_effect->setup(
        10 /* Line Min */,
        30, /* Line Max */
        10, /* Speed Min */
        25, /* Speed Max */
        60 /* Screen Update Interval */);
    return 0;
}

static void DigitalRain_process(AppController *sys,
                                const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // if (TURN_RIGHT == act_info->active)
    // {
    //     matrix_effect->setTextAnimMode(AnimMode::MATRIX, "\nWake Up, Neo...    \nThe Matrix has you.    \nFollow     \nthe white rabbit.     \nKnock, knock, Neo.", 20, 15);
    //     matrix_effect->setup(
    //         10 /* Line Min */,
    //         30, /* Line Max */
    //         5,  /* Speed Min */
    //         25, /* Speed Max */
    //         60 /* Screen Update Interval */);
    // }
    // if (TURN_LEFT == act_info->active)
    // {
    //     matrix_effect->setTextAnimMode(AnimMode::TEXT, "\nWake Up, Neo...    \nThe Matrix has you.    \nFollow     \nthe white rabbit.     \nKnock, knock, Neo.", 20, 15);
    // }
    matrix_effect->loop();

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 DigitalRain_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);
    // 程序需要时可以适当加延时
    // delay(30);
}

static void DigitalRain_background_task(AppController *sys,
                                        const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 DigitalRain_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 也可以移除自身的后台任务，放在本APP可控的地方最合适
    // sys->remove_backgroud_task();

    // 程序需要时可以适当加延时
    // delay(300);
}

static int DigitalRain_exit_callback(void *param)
{

    // 释放资源
    if (NULL != matrix_effect)
    {
        delete matrix_effect;
        matrix_effect = NULL;
    }
    setCpuFrequencyMhz(160);
    return 0;
}

static void DigitalRain_message_handle(const char *from, const char *to,
                                       APP_MESSAGE_TYPE type, void *message,
                                       void *ext_info)
{
}

APP_OBJ DigitalRain_app = {DIGITAL_RAIN_APP_NAME, &app_DigitalRain, "DigitalRain",
                           DigitalRain_init, DigitalRain_process, DigitalRain_background_task,
                           DigitalRain_exit_callback, DigitalRain_message_handle};
