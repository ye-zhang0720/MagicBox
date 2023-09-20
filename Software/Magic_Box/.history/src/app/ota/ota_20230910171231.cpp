#include <stdio.h>
#include "ota.h"
#include "ota_gui.h"
#include "sys/app_controller.h"
#include "app/app_conf.h"
#include "network.h"
#include "common.h"
#include <Update.h>

#define SERVER_REFLUSH_INTERVAL 5000UL // 配置界面重新刷新时间(5s)

/*
 * Server Index Page
 */

const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";
//  HTTPUpload& upload;
struct ServerAppRunData
{
    boolean web_start; // 标志是否开启web server服务，0为关闭 1为开启
    boolean ota_server_start;
    boolean req_sent;                     // 标志是否发送wifi请求服务，0为关闭 1为开启
    unsigned long serverReflushPreMillis; // 上一回更新的时间

    BaseType_t xReturned_ota_task_update; // 更新数据的异步任务
    TaskHandle_t xHandle_ota_task_update; // 更新数据的异步任务
    float upSize = 0;
    float totalSize = 0;
    int precent;
    // HTTPUpload& upload;
};

static ServerAppRunData *run_data = NULL;

static void start_ota_web_config()
{

    server.on("/", HTTP_GET, []()
              {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
    /*handling uploading firmware file */
    server.on(
        "/update", HTTP_POST, []()
        {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    AIO_LVGL_OPERATE_LOCK(close_ota_result_messageBox());
    ESP.restart(); },
        []()
        {
            // show_progress();
            HTTPUpload &upload = server.upload();
            run_data->ota_server_start = 1;
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
                run_data->upSize = (float)upload.totalSize;
                run_data->totalSize = (float)Update.size();
                run_data->precent = (int)(run_data->upSize * 100 / run_data->totalSize);
                 Serial.printf("Update  %d   %d   %d\n",  (int)upload.totalSize, (int) Update.progress(), run_data->precent);
                // if (upload.totalSize != 0)
                // {
                AIO_LVGL_OPERATE_LOCK(update_progress(run_data->precent););
                // }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                    AIO_LVGL_OPERATE_LOCK(show_ota_result_messageBox("更新成功, 正在重启"));
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                else
                {
                    Update.printError(Serial);
                    AIO_LVGL_OPERATE_LOCK(show_ota_result_messageBox("更新失败!!"));
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    AIO_LVGL_OPERATE_LOCK(close_ota_result_messageBox());
                }
            }
        });
    server.begin();
    Serial.println("HTTP server started");
}

static void stop_web_config()
{
    run_data->web_start = 0;
    run_data->req_sent = 0;
    run_data->ota_server_start = 0;
    server.stop();
    server.close();
}

void update_task_entry(void *p)
{
    start_ota_web_config();
    while (1)
    {
        server.handleClient(); // 一定需要放在循环里扫描
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

static int ota_init(AppController *sys)
{
    setCpuFrequencyMhz(240);
    ota_gui_init();
    // 初始化运行时参数
    run_data = (ServerAppRunData *)malloc(sizeof(ServerAppRunData));
    run_data->web_start = 0;
    run_data->req_sent = 0;
    run_data->ota_server_start = 0;
    run_data->serverReflushPreMillis = 0;
    run_data->xReturned_ota_task_update = xTaskCreate(
        update_task_entry,                   /*任务函数*/
        "update_task_entry",                 /*带任务名称的字符串*/
        6 * 1024,                            /*堆栈大小，单位为字节*/
        NULL,                                /*作为任务输入传递的参数*/
        2,                                   /*任务的优先级*/
        &run_data->xHandle_ota_task_update); /*任务句柄*/
    return 0;
}

static void ota_process(AppController *sys,
                        const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
  
        sys->app_exit();
        return;
    }

    if (0 == run_data->web_start && 0 == run_data->req_sent)
    {
        // 预显示
        display_ota(
            "访问下面ip地址, 升级MagicBox",
            "URL: MagicBox.local",
            "Wait...", "Wait...",
            // "", "",
            LV_SCR_LOAD_ANIM_NONE);
        sys->send_to(OTA_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        // 如果web服务没有开启 且 ap开启的请求没有发送 message这边没有作用（填0）
        sys->send_to(OTA_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_AP, NULL, NULL);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->web_start)
    {
        // server.handleClient(); // 一定需要放在循环里扫描
        // dnsServer.processNextRequest();
        if (doDelayMillisTime(SERVER_REFLUSH_INTERVAL, &run_data->serverReflushPreMillis, false) == true)
        {
            // 发送wifi维持的心跳
            sys->send_to(OTA_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);

            display_ota(
                "访问下面ip地址, 升级MagicBox",
                "URL: MagicBox.local",
                WiFi.localIP().toString().c_str(),
                WiFi.softAPIP().toString().c_str(),
                LV_SCR_LOAD_ANIM_NONE);
        }
    }
}

static void ota_background_task(AppController *sys,
                                const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int ota_exit_callback(void *param)
{

    ota_gui_del();
    // 查杀异步任务
    if (run_data->xReturned_ota_task_update == pdPASS)
    {
        vTaskDelete(run_data->xHandle_ota_task_update);
    }
    stop_web_config();
    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    
    setCpuFrequencyMhz(160);
    // vTaskDelay(100);
    // update_system_time();
    return 0;
}

static void ota_message_handle(const char *from, const char *to,
                               APP_MESSAGE_TYPE type, void *message,
                               void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_AP:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_ota(
            "访问下面ip地址, 设置APP",
            "URL: MagicBox.local",
            WiFi.localIP().toString().c_str(),
            WiFi.softAPIP().toString().c_str(),
            LV_SCR_LOAD_ANIM_NONE);
        // start_ota_web_config();
        run_data->web_start = 1;
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    default:
        break;
    }
}

APP_OBJ ota_app = {OTA_APP_NAME, &app_ota, "",
                   ota_init, ota_process, ota_background_task,
                   ota_exit_callback, ota_message_handle};
