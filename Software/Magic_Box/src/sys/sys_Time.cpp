#include "sys/sys_Time.h"
#include "ESP32Time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "app_controller.h"

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, NTP1, TIMEZERO_OFFSIZE, 60000);
ESP32Time sys_rtc;                    // 用于时间解码
extern AppController *app_controller; // APP控制器
SemaphoreHandle_t time_update_binary = xSemaphoreCreateBinary();

bool update_time_successed = false;


void update_system_time()
{
    xSemaphoreGive(time_update_binary);
}

void sys_rtc_update_task_Entry(void *p)
{
    unsigned long epochTime;
    struct tm *current_timeinfo;

    while (1)
    {
        if (pdTRUE == xSemaphoreTake(time_update_binary, (1000 * 60 * 60)))   // update time per hour 
        {
            app_controller->send_to(SYS_TIME_APP_NAME, CTRL_NAME,
                                    APP_MESSAGE_WIFI_CONN, NULL, NULL);
            update_time_successed = false;
            for(int i = 0; i < 20; i++)
            {
                if(WL_CONNECTED == WiFi.status())
                    break;
                vTaskDelay(500);
            }
            if (WL_CONNECTED == WiFi.status())
            {

                timeClient.begin();
                if(!timeClient.update())
                {
                    Serial.println("[EVENT] ERROR: updating time failed from NTP Server!!!");
                }
                if (timeClient.isTimeSet())
                {
                    // Get a time structure
                    epochTime = timeClient.getEpochTime();
                    current_timeinfo = gmtime((time_t *)&epochTime);
                    Serial.println(current_timeinfo);
                    sys_rtc.setTime(current_timeinfo->tm_sec, current_timeinfo->tm_min, current_timeinfo->tm_hour, current_timeinfo->tm_mday, current_timeinfo->tm_mon + 1, (current_timeinfo->tm_year + 1900));
                    // // Serial.println(sys_rtc.getTime("%A, %B %d %Y %H:%M:%S"));
                    update_time_successed = true;
                    // vTaskDelay(1000);
                    timeClient.end();
                }
            }else{
                Serial.println("[EVENT] ERROR: updating system time failed");
            }
        }else{
            update_system_time();
        }
    }
    vTaskDelete(NULL);
}

void sys_rtc_init()
{
    sys_rtc.setTime(0, 0, 0, 1, 1, 2023);
    xTaskCreate(sys_rtc_update_task_Entry, "sys_rtc_update_taskHandler", 2 * 1024, NULL, 2, NULL);
    update_system_time();
}
