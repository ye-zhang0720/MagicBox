#include "router_monitor.h"
#include "router_monitor_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include <ArduinoJson.h>
#include <string>
#include "network.h"
#define ROUTER_APP_NAME "路由器监控"

// 路由器的持久化配置
#define ROUTER_MONITOR_CONFIG_PATH "/router.cfg"

#define SERVER_REFLUSH_INTERVAL 5000UL // 配置界面重新刷新时间(5s)

struct routerConfig
{
    String NETDATA_HOST;
    int NETDATA_PORT;
    uint16_t mem_size;
    String CPUChartID;
    String memChartID;
    String networkChartID;
    String temperaturChartID;
};

struct RouterData
{
    int api;
    String id;
    String name;

    int view_update_every;
    int update_every;
    long first_entry;
    long last_entry;
    long before;
    long after;
    String group;
    String options_0;
    String options_1;

    JsonArray dimension_names;
    JsonArray dimension_ids;
    JsonArray latest_values;
    JsonArray view_latest_values;
    int dimensions;
    int points;
    String format;
    JsonArray result;
    double min;
    double max;
    bool wifi_status;
    unsigned long serverReflushPreMillis; // 上一回更新的时间
    double up_speed;
    double down_speed;
    double cpu_usage;
    double mem_usage;
    double temp_value;
};

static RouterData *run_data = NULL;
static routerConfig config;

static void write_config(routerConfig *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->NETDATA_HOST + "\n";
    w_data = w_data + cfg->CPUChartID + "\n";
    w_data = w_data + cfg->memChartID + "\n";
    w_data = w_data + cfg->networkChartID + "\n";
    w_data = w_data + cfg->temperaturChartID + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->NETDATA_PORT);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->mem_size);
    w_data += tmp;
    g_flashCfg.writeFile(ROUTER_MONITOR_CONFIG_PATH, w_data.c_str());
}

static void read_config(routerConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[1024] = {0};
    uint16_t size = g_flashCfg.readFile(ROUTER_MONITOR_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->NETDATA_HOST = "192.168.100.1";
        cfg->NETDATA_PORT = 19999;
        cfg->mem_size = 3839;
        cfg->CPUChartID = "system.cpu";
        cfg->memChartID = "mem.available";
        cfg->networkChartID = "net.eth0";
        cfg->temperaturChartID = "sensors.temp_thermal_zone2_thermal_thermal_zone2";
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[7] = {0};
        analyseParam(info, 7, param);
        // 默认值
        cfg->NETDATA_HOST = param[0];
        cfg->CPUChartID = param[1];
        cfg->memChartID = param[2];
        cfg->networkChartID = param[3];
        cfg->temperaturChartID = param[4];
        cfg->NETDATA_PORT = atol(param[5]);
        cfg->mem_size = atol(param[6]);
    }
}

void parseNetDataResponse(WiFiClient &client, RouterData *data)
{
    // Stream& input;

    DynamicJsonDocument doc(2048);

    DeserializationError error = deserializeJson(doc, client);

    if (error)
    {
        // Serial.print(F("deserializeJson() failed: "));
        // Serial.println(error.f_str());
        return;
    }

    data->api = doc["api"]; // 1
    data->id = doc["id"].as<String>();
    data->name = doc["name"].as<String>();
    data->view_update_every = doc["view_update_every"]; // 1
    data->update_every = doc["update_every"];           // 1
    data->first_entry = doc["first_entry"];             // 1691505357
    data->last_entry = doc["last_entry"];               // 1691505905
    data->after = doc["after"];                         // 1691505903
    data->before = doc["before"];                       // 1691505904
    data->group = doc["group"].as<String>();            // "average"

    data->options_0 = doc["options"][0].as<String>(); // "jsonwrap"
    data->options_1 = doc["options"][1].as<String>(); // "natural-points"

    data->dimension_names = doc["dimension_names"];
    data->dimension_ids = doc["dimension_ids"];
    data->latest_values = doc["latest_values"];
    data->view_latest_values = doc["view_latest_values"];

    data->dimensions = doc["dimensions"];      // 3
    data->points = doc["points"];              // 2
    data->format = doc["format"].as<String>(); // "array"

    JsonArray db_points_per_tier = doc["db_points_per_tier"];
    data->result = doc["result"];
    data->min = doc["min"]; // 2.1594684
    data->max = doc["max"]; // 3.7468776
}

/**
 * 从软路由NetData获取监控信息
 * ChartID:
 *  system.cpu - CPU占用率信息
 *  sensors.temp_thermal_zone0_thermal_thermal_zone - CPU 温度信息
 */

bool getNetDataInfoWithDimension(String chartID, RouterData *data, String dimensions_filter)
{

    // String reqRes = "/api/v0/data?chart=sensors.temp_thermal_zone0_thermal_thermal_zone0&format=json&points=9&group=average&gtime=0&options=s%7Cjsonwrap%7Cnonzero&after=-10";
    String reqRes = "/api/v1/data?chart=" + chartID + "&format=array&points=9&group=average&gtime=0&options=s%7Cjsonwrap%7Cnonzero&after=-2";
    reqRes = reqRes + "&dimensions=" + dimensions_filter;

    WiFiClient client;
    bool ret = false;

    // 建立http请求信息
    String httpRequest = String("GET ") + reqRes + " HTTP/0.1\r\n" + "Host: " + config.NETDATA_HOST.c_str() + "\r\n" + "Connection: close\r\n\r\n";

    // 尝试连接服务器
    if (client.connect(config.NETDATA_HOST.c_str(), config.NETDATA_PORT))
    {
        // 向服务器发送http请求信息
        client.print(httpRequest);
        // Serial.println("Sending request: ");
        // Serial.println(httpRequest);

        // 获取并显示服务器响应状态行
        String status_response = client.readStringUntil('\n');
        // Serial.print("status_response: ");
        // Serial.println(status_response);
        // 使用find跳过HTTP响应头
        if (client.find("\r\n\r\n"))
        {
            // Serial.println("Found Header End. Start Parsing.");
        }

        // 利用ArduinoJson库解析NetData返回的信息
        parseNetDataResponse(client, data);
        ret = true;
    }
    else
    {
        // Serial.println(" connection failed!");
    }
    // 断开客户端与服务器连接工作
    client.stop();
    return ret;
}

bool getNetDataInfo(String chartID, RouterData *data)
{
    return getNetDataInfoWithDimension(chartID, data, "");
}

void getCPUUsage()
{
    if (getNetDataInfo(config.CPUChartID, run_data))
    {
        // Serial.print("CPU Usage: ");
        // Serial.println(String(run_data->max).c_str());

        run_data->cpu_usage = run_data->max;
    }
}

void getMemoryUsage()
{
    if (getNetDataInfo(config.memChartID, run_data))
    {
        // Serial.print("Memory Available: ");
        // Serial.println(String(run_data->max).c_str());

        run_data->mem_usage = 100 * (1.0 - run_data->max / config.mem_size);
    }
}

void getNetworkReceived()
{
    if (getNetDataInfoWithDimension(config.networkChartID, run_data, "received"))
    {
        // Serial.print("Received: ");
        // Serial.println(String(run_data->max).c_str());

        run_data->down_speed = run_data->max / 8.0; // byte = 8 bit
        // run_data.down_speed_max = updateNetSeries(download_serise, down_speed);
        // lv_chart_set_points(chart, ser2, download_serise);
    }
}

void getNetworkSent()
{
    if (getNetDataInfoWithDimension(config.networkChartID, run_data, "sent"))
    {
        // Serial.print("Sent: ");
        // Serial.println(String(run_data->max).c_str());

        run_data->up_speed = -1 * run_data->max / 8.0;
        // up_speed_max = updateNetSeries(upload_serise, up_speed);
        // lv_chart_set_points(chart, ser1, upload_serise);
    }
}

void getTemperature()
{
    if (getNetDataInfo(config.temperaturChartID, run_data))
    {
        // Serial.print("Temperature: ");
        // Serial.println(String(run_data->max).c_str());

        run_data->temp_value = run_data->max;
    }
}

static int router_init(AppController *sys)
{
    router_gui_init();
    // 初始化配置的参数
    read_config(&config);
    run_data = (RouterData *)calloc(1, sizeof(RouterData));

    sys->send_to(ROUTER_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, NULL, NULL);

    return 0;
}

static void router_process(AppController *sys,
                           const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(ROUTER_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    if (WL_CONNECTED == WiFi.status())
    {
        setIP_address(WiFi.localIP().toString().c_str());
        getCPUUsage();
        getMemoryUsage();
        getTemperature();
        getNetworkReceived();
        getNetworkSent();
        AIO_LVGL_OPERATE_LOCK(updateData(run_data->up_speed, run_data->down_speed, run_data->cpu_usage, run_data->mem_usage, run_data->temp_value););
    }

    if (doDelayMillisTime(SERVER_REFLUSH_INTERVAL, &run_data->serverReflushPreMillis, false) == true)
    {
        // 发送wifi维持的心跳
        sys->send_to(ROUTER_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_ALIVE, NULL, NULL);
    }

    // 程序需要时可以适当加延时
    delay(500);
}

static void router_background_task(AppController *sys,
                                   const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(ROUTER_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 也可以移除自身的后台任务，放在本APP可控的地方最合适
    // sys->remove_backgroud_task();

    // 程序需要时可以适当加延时
    // delay(300);
}

static int router_exit_callback(void *param)
{
    // 释放资源
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    
    return 0;
}

static void router_message_handle(const char *from, const char *to,
                                  APP_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    // 目前主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "host"))
        {
            snprintf((char *)ext_info, 32, "%s", config.NETDATA_HOST.c_str());
        }
        else if (!strcmp(param_key, "port"))
        {
            snprintf((char *)ext_info, 32, "%d", config.NETDATA_PORT);
        }
        else if (!strcmp(param_key, "cpu"))
        {
            snprintf((char *)ext_info, 256, "%s", config.CPUChartID.c_str());
        }
        else if (!strcmp(param_key, "mem"))
        {
            snprintf((char *)ext_info, 256, "%s", config.memChartID.c_str());
        }
        else if (!strcmp(param_key, "network"))
        {
            snprintf((char *)ext_info, 256, "%s", config.networkChartID.c_str());
        }
        else if (!strcmp(param_key, "temperature"))
        {
            snprintf((char *)ext_info, 256, "%s", config.temperaturChartID.c_str());
        }
        else if (!strcmp(param_key, "mem_size"))
        {
            snprintf((char *)ext_info, 32, "%d", config.mem_size);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "host"))
        {
            config.NETDATA_HOST = param_val;
        }
        else if (!strcmp(param_key, "port"))
        {
            config.NETDATA_PORT = atol(param_val);
        }
        else if (!strcmp(param_key, "cpu"))
        {
            config.CPUChartID = param_val;
        }
        else if (!strcmp(param_key, "mem"))
        {
            config.memChartID = param_val;
        }
        else if (!strcmp(param_key, "network"))
        {
            config.networkChartID = param_val;
        }
        else if (!strcmp(param_key, "temperature"))
        {
            config.temperaturChartID = param_val;
        }
        else if (!strcmp(param_key, "mem_size"))
        {
            config.mem_size = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&config);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&config);
    }
    break;
    default:
        break;
    }
}

APP_OBJ router_app = {ROUTER_APP_NAME, &app_router, "Author XBPB\nVersion 1.0.0\n",
                      router_init, router_process, router_background_task,
                      router_exit_callback, router_message_handle};
