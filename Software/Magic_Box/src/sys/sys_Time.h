#ifndef SYS_TIME_H
#define SYS_TIME_H

#include "network.h"
#include "common.h"
#include <WiFi.h>
#include <esp_wifi.h> //用于esp_wifi_restore() 删除保存的wifi信息

#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp.tencent.com"
#define NTP4 "pool.ntp.org"

#define SYS_TIME_APP_NAME "system_time"

extern bool update_time_successed;
void sys_rtc_init();
void update_system_time();

#endif