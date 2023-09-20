#ifndef APP_OTA_H
#define APP_OTA_H

#include "sys/interface.h"
#define OTA_APP_NAME "系统更新"

#ifdef ESP8266
#include <ESP8266WiFi.h>      // Built-in
#include <ESP8266WiFiMulti.h> // Built-in
#include <ESP8266WebServer.h> // Built-in
#include <ESP8266mDNS.h>
extern ESP8266WebServer server;
#else
#include <WiFi.h>      // Built-in
#include <WiFiMulti.h> // 当我们需要使用ESP8266开发板存储多个WiFi网络连接信息时，可以使用ESP8266WiFiMulti库来实现。
#include <WebServer.h> // https://github.com/Pedroalbuquerque/ESP32WebServer
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <HTTPClient.h>
extern WebServer server;
#endif

extern APP_OBJ ota_app;

#endif