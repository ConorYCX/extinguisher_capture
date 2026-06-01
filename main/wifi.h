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
#ifndef wifi_h_
#define wifi_h_

#ifndef wifi_c_
#define wifi_cx_ extern
#else
#define wifi_cx_
#endif


#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/ringbuf.h"
#include "esp_private/esp_clk.h"
#include "soc/soc.h"
#include "hal/efuse_hal.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_spi_flash.h"

#include "soc\rtc.h"
#include "driver\uart.h"
#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"

#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "audio_camera.h"

//WIFI_MODE_STA:连接路由器模式;    WIFI_MODE_AP:开热点模式;    WIFI_MODE_APSTA:同时开热点和连接路由器                                                
#define default_wifi_mode      WIFI_MODE_STA  //模式 0-WIFI_MODE_NULL; 1-WIFI_MODE_STA; 2-WIFI_MODE_AP; 3-WIFI_MODE_APSTA

#define default_wifi_ap_ssid      "audioCameraWifi"  //设置模块发出的无线名称
#define default_wifi_ap_pass      "11223344"  //无线密码(无密码则为 "")
#define default_wifi_ap_channel   11  //信道
#define default_wifi_ap_max_count  5  //最大连接数(最大10个)
#define default_wifi_ap_ssid_add_mac 0//是否设置无线名称后缀增加模组的MAC地址信息(名称动态唯一); 1:设置 0:不设置

#define default_wifi_ap_4g_wifi_enable 0 //是否打开4G热点,随身wifi功能


#define default_wifi_sta_ssid       "28-232-2.4G"     //设置模块连接的路由器名称
#define default_wifi_sta_pass       "hdu028232"  //路由器密码(无密码则为 "")

//静态IP
#define default_wifi_sta_ip       ""  //设置静态IP, 如果不设置保持 ""
#define default_wifi_sta_netmask  "255.255.255.0"    //子网掩码
#define default_wifi_sta_gw       "192.168.168.140"  //网关地址


//WiFi
typedef struct{
    char sta_ip[36];       //设置静态IP
    char sta_netmask[36];  //子网掩码
    char sta_gw[36];       //网关地址

    char mode;//AP 或者 STA模式
    char ap_ssid[32];//热点名称
    char ap_pass[64];//热点密码
    char ap_channel;//通道
    char ap_max_count;//最大连接数
    char ap_ssid_add_mac;//是否在热点名称追加唯一MAC字符串,让热点唯一

    char ap_4g_wifi_enable;//是否打开4G热点,随身wifi功能

    char sta_ssid[32];//连接的路由器名称
    char sta_pass[64];//密码
}parame_config_wifi_t;

wifi_cx_ parame_config_wifi_t parame_config_wifi;


typedef struct{
  uint8_t sta_state;//0-未连接路由器  1-连接上路由器但没分配到IP  2-连接上路由器并分配到IP
  ip_event_got_ip_t* event;//可以获取到连接的路由器信息,和模组分配的IP地址
}wifi_struct_t;

wifi_cx_ wifi_struct_t wifi_struct;

void wifi_init_softap_sta(parame_config_wifi_t *wifi);

void wifi_init(void);

#endif


