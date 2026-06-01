/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "esp_sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "mqtt_client.h"

#include "audio_camera.h"
#include "esp_http_client.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "gy_gpio.h"
#include "demo.h"

#define TAG "main"

#define gpio_init_pin 0
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());//初始化内部的lwip
    ESP_ERROR_CHECK(esp_event_loop_create_default());//创建系统事件任务

    
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());

    //设置型号
    gy_set_device_model(CAM_A1);
    
    /*配置GPIO0输出低电平用来给摄像头和内存卡供电 必须提前配置这个,否则下面会初始化失败*/
    gy_gpio_setup(gpio_init_pin, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_ONLY, GPIO_INTR_MAX, NULL);//设置GPIO为输入输出模式
    gpio_set_level(gpio_init_pin, 0);//输出低电平
    
    gy_gpio_setup(46, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_ONLY, GPIO_INTR_MAX, NULL);//设置GPIO为输入输出模式
    gpio_set_level(46, 0);//初始化关闭补光灯

    
    demo_test();//整板测试

    while (1)
    {   
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
