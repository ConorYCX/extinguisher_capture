/********************************************************************************
  * @file
  * @author  fengwu yang
	* @date    2024/4/1
    * @version V1.0.0
    * 初版
    
  * @brief   
  ******************************************************************************

  ******************************************************************************
  */
#ifndef demo_h_
#define demo_h_

#ifndef demo_c_
#define demo_cx_ extern
#else
#define demo_cx_
#endif


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
#include "esp_sleep.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "wifi.h"
#include "gy_gpio.h"

#if(1)
#define debug_printf ESP_LOGI
#else
#define debug_printf(...)
#endif

/*队列传输数据通用模板******************************************************************************************************************/

#define QueueHandleQueueLength 500

#define QueueHandleQueueLength 500

//数据的一些含义
typedef enum {
    data_mqtt             = 0,//从mqtt过来的数据
    data_mqtt_connected,   //mqtt连接上服务器
    data_usb,  //从usb过来的数据
    data_http,  //从http过来的数据
}enum_data_type;

typedef struct{
  enum_data_type data_type;
  char *data;
  size_t data_len;
}struct_queue_data;


demo_cx_ QueueHandle_t QueueHandle;

void queue_data_init(int Priority, uint32_t StackDepth);

/******************************************************************************************************************队列传输数据通用模板*/

void demo_test(void);



#endif