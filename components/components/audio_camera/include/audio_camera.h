#ifndef camera_init_h_
#define camera_init_h_

#ifndef camera_init_c_
#define camera_init_cx_ extern
#else
#define camera_init_cx_
#endif


/**
  ******************************************************************************
  * @file    audio_camera
  * @author  feng wu yang						
	* @version V0.0.1
	1.增加http form-data提交数据

  * @version V1.0.0
	1.增加OTA 升级

  * @version V2.0.0
	1.增加MQTT
  
  * @version V2.1.0
	1.增加TCP 客户端
  2.修改OTA升级BUG(内存不足导致无法退出OTA升级)

  * @version V3.0.0
	1.为了统一底层框架, 增加设备型号设置函数;
  2.优化了摄像头采集程序;
  3.增加了4G热点,随身wifi功能
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "esp_camera.h"
#include "esp_random.h"
#include "esp_http_client.h"

#include "usbh_modem_board.h"
#include "usbh_modem_wifi.h"

#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/dns.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/i2c.h"


//设置板子型号
/*板子型号********************************************************************************************************************************/
typedef enum {
    CAM_A0= 0,//摄像头, A0 版
    CAM_A1,   //摄像头, A1 版
    CAM_A2,   //摄像头, A2 版

    CAM_MAX,
}struct_device_model;

void gy_set_device_model(struct_device_model model);//设置设备型号
/********************************************************************************************************************************板子型号*/


/*(传感器)********************************************************************************************************************************/

/*获取板载的温湿度数据-支持版本(CAM_A2,)
return: 0-成功; 1-启动测量命令错误; 2-获取温湿度数据错误; 3-数据CRC校验错误*/
uint8_t gy_get_temp_humidity(float* temperature, float* humidity);

/********************************************************************************************************************************(传感器)*/



/*sleep********************************************************************************************************************************/

typedef enum {
    WAKEUP_TYPE_LOW_LEVEL= 0,//低电平唤醒
    WAKEUP_TYPE_HIGH_LEVEL,  //高电平唤醒
    WAKEUP_TYPE_POSEDGE,   //上升沿
    WAKEUP_TYPE_NEGEDGE,   //下降沿
    WAKEUP_TYPE_ANYEDGE,   //上升沿,下降沿 (不支持)
    
    WAKEUP_TYPE_MAX,
}struct_gpio_wakeup_type;

/*设置某个GPIO在休眠时保持高低电平*/
void gy_sleep_gpio_hold(gpio_num_t gpio_num, uint32_t level);

//配置引脚唤醒: gpio_num: 设置使用哪个GPIO唤醒设备; struct_gpio_wakeup_type: 唤醒方式
void gy_sleep_gpio_wakeup(gpio_num_t gpio_num, struct_gpio_wakeup_type gpio_wakeup_type);

/*取消GPIO的唤醒功能(使用唤醒引脚做其它用途,需要先调用这个)*/
void gy_sleep_gpio_deinit(gpio_num_t gpio_num);

/*休眠多少us*/
void gy_sleep_timer(uint64_t time_in_us);

/*进入深度睡眠模式*/
void gy_sleep_deep(void);
/********************************************************************************************************************************sleep*/



/*TCP Client********************************************************************************************************************************/
typedef void (* tcp_client_callback)(void *arg);
typedef void (* tcp_client_callback1)(void *arg, char *data, size_t len);

typedef struct
{
    char ip[130];
    int port;
    //回调函数
    tcp_client_callback client_connected_cb;//客户端连接回调函数
    tcp_client_callback client_disconnected_cb;//客户端断开回调函数
    tcp_client_callback1 client_recv_cb;//客户端接收回调函数

    int local_port;//本地端口号 0-自动分配
    size_t  task_stack;//任务堆栈大小
    uint8_t task_priority;//任务优先级
    xTaskHandle xTaskHandle_t;//记录任务句柄

    //保活
    char keepLiveEnable;//使能
    u32_t keepIdle;//N毫秒内双方无数据则发起保活探测
    u32_t keepCnt;//N毫秒发送一次保活探测
    u32_t keepIntvl;//N次探测无响应则断开

    struct netconn *netconn_c;
    char reconnect;
    char connected;//1连接, 0断开

    err_t disconret;//记录断开原因
}struct_tcp_client_t;


void gy_tcp_init(struct_tcp_client_t *struct_tcp_client);

//连接服务器; 0:成功; -1:正在连接
char gy_tcp_client_connect(struct_tcp_client_t *struct_tcp_client);

//关闭连接; 0:成功; -1:未连接
char gy_tcp_client_close(struct_tcp_client_t *struct_tcp_client);
/********************************************************************************************************************************TCP Client*/





/*OTA******************************************************************************************************************/
typedef enum {
    enum_ota_start             = 0,//开始
    enum_ota_err_http_init,         /*http 初始化失败*/
    enum_ota_err_http_open,         /*http 打开失败*/
    enum_ota_err_malloc,           /*http 申请内存失败*/
    enum_ota_err_http_read,        /*http 读取数据错误*/
    enum_ota_firmware_version_same,    /*固件版本相同*/
    enum_ota_err_begin      ,  /*准备写入新固件失败*/
    enum_ota_begin       ,  /*开始写入新固件*/
    enum_ota_err_first_data,  /*接收第一包数据错误*/
    enum_ota_err_write ,  /*写数据错误*/
    enum_ota_write ,  /*正在循环下载和往flash写数据*/
    enum_ota_err_http_close,  /*http 异常断开*/
    enum_ota_http_close,  /*接收完数据http断开*/
    enum_ota_err_recv_0,  /*接收返回0长度数据, 有可能底层缓存不够,需要等到有缓存以后才会接着接收*/
    enum_ota_err_data_loss,  /*接收数据不完整*/
    enum_ota_err_end,  /*最后保存固件失败*/
    enum_ota_err_run_new ,  /*设置启动新的固件失败*/
    enum_ota_restart  ,  /*OTA整个流程执行完,重启*/
}struct_ota_state;


typedef char (* gy_ota_callback)(struct_ota_state ota_state, char code, char *data, size_t data_len);
typedef struct{
  uint8_t runing;
  gy_ota_callback ota_callback;//回调函数

  size_t  task_stack;//任务堆栈大小
  uint8_t task_priority;//任务优先级
  xTaskHandle xTaskHandle_t;//记录任务句柄

  esp_http_client_config_t http_client_config;//配置http
}struct_ota_task_t;


#define define_gy_ota_header_size (sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t) + 1)
#define define_gy_ota_buff_size   (define_gy_ota_header_size+1024)  //设置每次请求的数据大小


esp_err_t gy_ota_get_state_run(esp_ota_img_states_t *ota_state);//检查运行区程序

void gy_ota_get_sha256_table(uint8_t *sha_256);//获取分区表的sha256
void gy_ota_get_sha256_boot(uint8_t *sha_256);//获取boot程序的sha256
void gy_ota_get_sha256_run(uint8_t *sha_256);//获取当前运行程序的sha256

void gy_ota_task_create(struct_ota_task_t *struct_ota_task);

esp_err_t gy_ota_get_version_running(char *data, char data_len);//获取当前运行的版本

/******************************************************************************************************************OTA*/



/*camera********************************************************************************************************************************/
esp_err_t gy_camera_init(camera_config_t *camera_config);
uint8_t   gy_camera_focus_init(sensor_t *sensor); //对焦初始化
uint8_t   gy_camera_focus_enable(sensor_t *sensor); //启动对焦
uint8_t   gy_camera_focus_tatus(sensor_t *sensor); //判断是否完成对焦
/********************************************************************************************************************************camera*/

esp_err_t gy_4g_init(esp_event_handler_t handler);
void      gy_4g_wifi_enable(esp_netif_t *netif);

/*
parameter: buff(cached data)  buff_len(cached data buff length)  err_log_en(1:printf err log)
return: 0,success; -1,content_type is not support
*/
int gy_http_post_form_data_start(esp_http_client_handle_t client, char *buff, uint32_t buff_len, char err_log_en);

/*
return: 0,success; -1,buff is NULL for gy_http_post_init;  -2,buff is full
*/
int gy_http_post_form_data_body(esp_http_client_handle_t client, char *key, char *filename, char *content_type, char *data, uint32_t data_len, char err_log_en);

/*
return: 0,success; -2,buff is full
*/
int gy_http_post_form_data_stop(esp_http_client_handle_t client, char err_log_en);


#endif

/*
一、文本类型
text/plain：纯文本格式，不包含任何格式或样式信息。
text/html：HTML格式，用于网页内容的展示。
text/css：CSS格式，用于定义网页的样式和布局。
text/javascript 或 application/javascript：JavaScript格式，用于网页的脚本编写和动态交互。
text/xml 或 application/xml：XML格式，用于数据的结构化表示和传输。
二、图像类型
image/gif：GIF图片格式，支持动画和透明背景。
image/jpeg 或 image/jpg：JPEG图片格式，一种常用的有损压缩图片格式。
image/png：PNG图片格式，支持无损压缩和透明背景。
image/bmp：BMP图片格式，Windows操作系统中的标准图像格式。
image/vnd.microsoft.icon 或 image/ico：ICO图标格式，常用于网站和应用程序的图标。
三、音频类型
audio/mpeg 或 audio/mp3：MP3音频格式，一种广泛使用的音频压缩格式。
audio/aac：AAC音频格式，一种高级音频编码格式，提供比MP3更好的音质。
audio/ogg：OGG音频格式，一种开源的音频压缩格式，支持多声道和高质量音频。
audio/midi 或 audio/x-midi：MIDI音频格式，用于表示音乐乐器数字接口（MIDI）数据。
四、视频类型
video/mpeg：MPEG视频格式，一种常用的视频压缩格式。
video/mp4：MP4视频格式，一种广泛使用的视频格式，支持高质量视频和音频的编码。
video/ogg：OGG视频格式，与OGG音频格式类似，但用于视频数据的编码。
video/x-msvideo：AVI视频格式，一种旧的视频格式，但仍然在某些情况下使用。
五、应用程序类型
application/pdf：PDF格式，用于电子文档的表示和传输。
application/msword：Microsoft Word文档格式。
application/vnd.openxmlformats-officedocument.wordprocessingml.document：Microsoft Word的OpenXML格式（.docx）。
application/json：JSON格式，一种轻量级的数据交换格式，常用于Web API的数据传输。
application/zip：ZIP压缩格式，用于文件的压缩和归档。
application/x-www-form-urlencoded：表单数据编码格式，默认用于HTML表单的提交。
multipart/form-data：用于在表单中进行文件上传时的编码格式。
六、其他类型
application/octet-stream：二进制流数据格式，常用于文件下载。
application/x-abiword：AbiWord文档格式。
application/x-freearc：ARC压缩文档格式。
application/vnd.amazon.ebook：Amazon Kindle电子书格式。
application/vnd.apple.installer+xml：Apple安装程序包格式。
application/vnd.oasis.opendocument.（后跟具体类型，如presentation、spreadsheet、text）：OpenDocument格式，用于表示OpenOffice、LibreOffice等办公软件的文档。
*/

