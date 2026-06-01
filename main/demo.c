#define demo_c_
#include "demo.h"


#define TAG "demo"


#define gpio_camera_power_pin 0 //控制摄像头的电源引脚

int32_t on_modem_event_id = MODEM_EVENT_UNKNOWN;


//操作nvs时用的句柄
nvs_handle_t my_nvs_handle;
//设置休眠时间
uint64_t sleep_time_ok=60;//正常休眠时间(分钟)
uint64_t sleep_time_err=30;//错误休眠时间(分钟)

//时间
time_t time1 = 0;
time_t time_stamp;//时间戳
struct tm timeinfo = { 0 };


#define sleep_gpio_pin         GPIO_NUM_18  //设置唤醒引脚,不使用可设置为 -1
#define sleep_gpio_wakeup_type WAKEUP_TYPE_LOW_LEVEL  //设置唤醒方式

//sleep_time:休眠时间(us); gpio_num:使用哪个引脚唤醒(GPIO0-GPIO21);  gpio_wakeup_type:高电平或低电平唤醒
void sleep_time_gpio_wakeup(uint64_t sleep_time, gpio_num_t gpio_num, struct_gpio_wakeup_type gpio_wakeup_type) 
{
    //使能一个GPIO作为唤醒引脚(可选设置)
    if (gpio_num>=0)
    {
        gy_sleep_gpio_wakeup(gpio_num, gpio_wakeup_type);//设置GPIO18 低电平唤醒
        //可以多设置几个唤醒IO
        //gy_sleep_gpio_wakeup(gpio_num, gpio_wakeup_type);//设置GPIO18 低电平唤醒
    }
    if (sleep_time>0)
    {
        //使能定时唤醒 us(可选设置)
        gy_sleep_timer(sleep_time*1000*1000);//设置定时唤醒
    }
    ESP_LOGI(TAG, "sleep sleep sleep sleep sleep.....");
    gy_sleep_deep();//进入深度休眠模式
}



/*队列传输数据通用模板******************************************************************************************************************/
SemaphoreHandle_t SemaphoreHandleQueueHandleTaskSend;

//把数据发送到队列
void queue_data_send(void *data, size_t data_len,enum_data_type data_type)
{
    BaseType_t ret = 0;

    ret = xSemaphoreTake(SemaphoreHandleQueueHandleTaskSend, portMAX_DELAY);
    if (ret == pdTRUE)
    {
        /*把数据发送到队列*/
        struct_queue_data* queue_data = (struct_queue_data*)malloc(sizeof(struct_queue_data));
        if (queue_data!=0)
        {
            // if (data_type == data_http)
            {
                char *malloc_buff = (char *) malloc(data_len+1);//注意这里申请内存一定要比实际数据个数+1

                if (malloc_buff!=0)
                {
                    memcpy(malloc_buff, data, data_len);

                    queue_data->data = malloc_buff;
                    queue_data->data_len = data_len;
                    
                    queue_data->data_type = data_type;
                
                    //把消息发送到消息队列
                    if (xQueueSend(QueueHandle, &queue_data, 0) != pdPASS)
                    {
                        free(queue_data);
                        free(malloc_buff);
                        ESP_LOGE(TAG, "xQueueSend(QueueHandle, &queue_data, 0)  ERR");
                    }
                }
                else
                {
                    free(queue_data);
                    ESP_LOGE(TAG, "char *p_data1 = (char *) malloc(event->data_len+1) ERR");
                }  
            }
        }
        else
        {
            ESP_LOGE(TAG, "struct_queue_data* queue_data = (struct_queue_data*)malloc(sizeof(struct_queue_data)) ERR");
        }
    }
    xSemaphoreGive(SemaphoreHandleQueueHandleTaskSend);
}


static void QueueHandleTask(void *arg)
{
    struct_queue_data *queue_data;
    int msg_id=0;
    int len=0;
    char *malloc_buff = (char *) malloc(1501);
    cJSON *json_parse,*json_value;
    char *json_data = (char *) malloc(100);

    char sleep_flag=0;
    for (;;)
    {
        if(xQueueReceive(QueueHandle, &queue_data, portMAX_DELAY))//取消息队列数据
        {
            len = queue_data->data_len;
            memcpy(malloc_buff, queue_data->data, len);
            malloc_buff[queue_data->data_len]=0;

            ESP_LOGI(TAG, "QueueHandleTask=%d",queue_data->data_type);
            ESP_LOGI(TAG, "QueueHandleTask=%s",malloc_buff);
            
            if (queue_data->data_type == data_http)
            {
                //{"code":"OK","imei":"xxxxxxxxxxxxxxx","time":"2025-07-22 23:07:21","vbat":"4.20","rssi":"26","sleep_time_ok":60,"sleep_time_err":30}
                json_parse = cJSON_Parse(malloc_buff);//把数据存储到cJSON链表中
                if(json_parse)//是JSON数据
                {
                    json_value = cJSON_GetObjectItem(json_parse,"code");//查找字段
                    if(json_value!=NULL && json_value->type == cJSON_String)//获取当前字段的数据类型
                    {
                        memset(json_data, 0, 100);
                        memcpy(json_data, json_value->valuestring, strlen(json_value->valuestring));

                        ESP_LOGI(TAG, "json_data=%s",json_data);//打印数据
                    }
                    
                    //获取正常运行时的休眠时间
                    json_value = cJSON_GetObjectItem(json_parse,"sleep_time_ok");//查找字段
                    if(json_value!=NULL && json_value->type == cJSON_Number)//获取当前字段的数据类型
                    {
                        ESP_LOGI(TAG, "sleep_time_ok=%d\r\n",json_value->valueint);//打印相应字段的值

                        sleep_time_ok = json_value->valueint;
                        /*打开*/     //操作的表格名字 //以读写模式打开
                        nvs_open("storage", NVS_READWRITE, &my_nvs_handle);
                        nvs_set_i32(my_nvs_handle, "sleep_time_ok", sleep_time_ok);
                        nvs_commit(my_nvs_handle);
                        nvs_close(my_nvs_handle);
                    }

                    //获取运行失败时的休眠时间
                    json_value = cJSON_GetObjectItem(json_parse,"sleep_time_err");//查找字段
                    if(json_value!=NULL && json_value->type == cJSON_Number)//获取当前字段的数据类型
                    {
                        ESP_LOGI(TAG, "sleep_time_err=%d\r\n",json_value->valueint);//打印相应字段的值
                        
                        sleep_time_err = json_value->valueint;
                        /*打开*/     //操作的表格名字 //以读写模式打开
                        nvs_open("storage", NVS_READWRITE, &my_nvs_handle);
                        nvs_set_i32(my_nvs_handle, "sleep_time_err", sleep_time_err);
                        nvs_commit(my_nvs_handle);
                        nvs_close(my_nvs_handle);
                    }

                    //设置休眠
                    sleep_time_gpio_wakeup(sleep_time_ok*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                }
                if(json_parse) cJSON_Delete(json_parse);//释放内存
            }
            
            free(queue_data->data);
            free(queue_data);
        }
    }
    vTaskDelete(NULL);
}


QueueHandle_t QueueHandle;
void queue_data_init(int Priority, uint32_t StackDepth)//队列传输数据通用模板
{   
    QueueHandle = xQueueCreate(QueueHandleQueueLength, sizeof(struct_queue_data *)); //创建队列   
    xTaskCreate(QueueHandleTask, "QueueHandleTask", StackDepth, NULL, Priority, NULL);

    SemaphoreHandleQueueHandleTaskSend = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaphoreHandleQueueHandleTaskSend);
}
/******************************************************************************************************************队列传输数据通用模板*/



/*摄像头配置参数*************************************************************************************************************************************/

#define amera_focus_enable 0//是否支持自动对焦

camera_config_t camera_config_init = {
    .xclk_freq_hz = 8000000,
    .pixel_format = PIXFORMAT_JPEG, //PIXFORMAT_JPEG, PIXFORMAT_YUV422, PIXFORMAT_GRAYSCALE, PIXFORMAT_RGB565, PIXFORMAT_JPEG
    .frame_size = FRAMESIZE_QSXGA,    //设置照片分辨率
    .jpeg_quality = 5, //压缩率(取值范围0-63);  值越小,图片质量越好
    .fb_count = 2,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};


/*SD卡配置参数*************************************************************************************************************************************/
#define MOUNT_POINT "/sdcard"
#define PIN_NUM_MISO  21
#define PIN_NUM_MOSI  41
#define PIN_NUM_CLK   42

const char mount_point[] = MOUNT_POINT;
sdmmc_card_t *card=NULL;
esp_err_t mount_ret = ESP_OK;
esp_err_t sdcard_init()
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
            .format_if_mount_failed = true,
    #else
        .format_if_mount_failed = false,
    #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    if (card!=NULL)
    {
        if (mount_ret == ESP_OK)
        {
            esp_vfs_fat_sdcard_unmount(mount_point, card);          /* 取消挂载 */
        }
        //deinitialize the bus after all devices are removed
        spi_bus_free(host.slot);
    }
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = 0xFF;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    mount_ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);/* 挂载文件系统 */

    if (mount_ret != ESP_OK) {
        if (mount_ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return mount_ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    return mount_ret;
}



/*****配置和4G模组相连接的串口************************************************************************************************************************************/

#define uart_init_4g_port UART_NUM_1

void uart_init_4g()
{
    /*配置串口参数*/
    uart_config_t uart_config = {
        .baud_rate = 115200,//波特率
        .data_bits = UART_DATA_8_BITS,//数据位8位
        .parity    = UART_PARITY_DISABLE,//无奇偶校验
        .stop_bits = UART_STOP_BITS_1,//停止位1位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,//不使用硬件流控
        .source_clk = UART_SCLK_APB,//串口使用的时钟
    };
    /*初始化串口1*/
    uart_driver_install(uart_init_4g_port,
        1024, //串口1接收缓存大小
        1024,
        0, //队列大小为0;没有使用freertos内部缓存管理
        NULL, //不使用QueueHandle_t 内部缓存管理,设置为空
        0 //设置串口中断优先级,设置为0意味着让系统从1-3级中自动选择一个
    );
    /*设置串口参数*/
    uart_param_config(uart_init_4g_port, &uart_config);
    /*设置串口的TX,RX,RTS,DTR引脚*/             //不使用RTS,DTR
    uart_set_pin(uart_init_4g_port, 48, 47, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

float modem_vbat_value=0;//电池电压
double modem_latitude=-1, modem_longitude=-1;//基站定位经纬度

int modem_year=0;//年
int modem_month=0;
int modem_day=0;
int modem_hour=0;
int modem_minute=0;
int modem_second=0;


static void uart_init_4g_task(void *pvParameters)
{
    char *uart_recv_data = (char *) malloc(500);
    int len=0;
    float float_value;
    int int_value;

    struct tm struct_tm={0};

    uart_init_4g();/*配置和4G模组相连接的串口*/
    while (1)
    {
        len=0;
        memset(uart_recv_data,0,500);
        
        if (modem_vbat_value==0)//获取电池电压
        {
            ESP_LOGE(TAG, "AT+CBC\n");
            uart_write_bytes(uart_init_4g_port, (const char *) "AT+CBC\r\n", strlen("AT+CBC\r\n"));//+CBC: <volt>
            memset(uart_recv_data, 0, 500); len = uart_read_bytes(uart_init_4g_port, uart_recv_data, 500, 100 / portTICK_RATE_MS);
            if (len>0)
            {
                if (sscanf(uart_recv_data, "AT+CBC\r\n\r\n+CBC: %d", &int_value) == 1) {  
                    
                    modem_vbat_value = int_value;
                    modem_vbat_value = modem_vbat_value/1000;

                    ESP_LOGI(TAG, "modem_vbat_value:%f\n", modem_vbat_value);//打印获取的电压
                } else {  
                    ESP_LOGE(TAG, "modem_vbat_value err\n");  
                }
            }
        }
        else if (modem_year == 0)//获取时间
        {
            uart_write_bytes(uart_init_4g_port, (const char *) "AT+CCLK?\r\n", strlen("AT+CCLK?\r\n"));//
            memset(uart_recv_data, 0, 500); len = uart_read_bytes(uart_init_4g_port, uart_recv_data, 500, 100 / portTICK_RATE_MS);
            if (len>0)
            {
                if (len>0) ESP_LOGI(TAG, "uart_recv_data:%s\n", uart_recv_data);//打印模组返回的数据

                if (sscanf(uart_recv_data, "AT+CCLK?\r\n\r\n+CCLK: \"%d/%d/%d,%d:%d:%d+%*s", &modem_year,&modem_month,&modem_day,&modem_hour,&modem_minute,&modem_second) == 6) 
                {
                    struct_tm.tm_year = modem_year+2000-1900;
                    struct_tm.tm_mon = modem_month-1;
                    struct_tm.tm_mday = modem_day;
                    struct_tm.tm_hour = modem_hour;
                    struct_tm.tm_min = modem_minute;
                    struct_tm.tm_sec = modem_second;

                    // 转换为时间戳
                    time_t timestamp = mktime(&struct_tm);
                    struct timeval tv = {
                        .tv_sec = timestamp,
                        .tv_usec = 0
                    };
                    
                    // 设置系统时间，同时会更新RTC
                    settimeofday(&tv, NULL);
                    ESP_LOGI(TAG, "RTC时间已设置");
                    ESP_LOGI(TAG,"%d,%d,%d, %d,%d,%d", modem_year,modem_month,modem_day,modem_hour,modem_minute,modem_second);
                } 
                else
                {  
                    ESP_LOGE(TAG, "AT+CCLK err\n");  
                }
            }
        }
        else if (modem_latitude==-1 || modem_longitude==-1)//获取定位
        {
            #if 0 //使用GPS定位
                uart_write_bytes(uart_init_4g_port, (const char *) "AT+CGNSPWR?\r\n", strlen("AT+CGNSPWR?\r\n"));//开启定位
                memset(uart_recv_data, 0, 500); len = uart_read_bytes(uart_init_4g_port, uart_recv_data, 500, 100 / portTICK_RATE_MS);
                if (len>0) ESP_LOGI(TAG, "uart_recv_data:%s\n", uart_recv_data);//打印模组返回的数据
                if (len>0)
                {
                    if (strstr(uart_recv_data, "+CGNSPWR: 0"))
                    {
                        ESP_LOGI(TAG, "start gps power\n");
                        vTaskDelay(pdMS_TO_TICKS(20));
                        uart_write_bytes(uart_init_4g_port, (const char *) "AT+CGNSPWR=1\r\n", strlen("AT+CGNSPWR=1\r\n"));//开启定位
                        vTaskDelay(pdMS_TO_TICKS(100));
                        uart_write_bytes(uart_init_4g_port, (const char *) "AT+CGNSAID=31,1,1,1\r\n", strlen("AT+CGNSAID=31,1,1,1\r\n"));//开启辅助定位
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    else
                    {
                        uart_write_bytes(uart_init_4g_port, (const char *) "AT+CGNSINF\r\n", strlen("AT+CGNSINF\r\n"));//获取GPS数据
                        memset(uart_recv_data, 0, 500); len = uart_read_bytes(uart_init_4g_port, uart_recv_data, 500, 100 / portTICK_RATE_MS);
                        if (len>0) ESP_LOGI(TAG, "uart_recv_data:%s\n", uart_recv_data);//打印模组返回的数据

                        if (len>0)
                        {//+CIPGSMLOC: 0,22.7531414,108.3727417,2024/12/02,01:16:48
                        //AT+CGNSINF\r\n\r\n+CGNSINF: 1,1,20250726151015,22.758355,113.822893,27.299,0.00,0.00,3,,3.47,3.41,4.00,,14,10,,,44,,
                            if (sscanf(uart_recv_data, "AT+CGNSINF\r\n\r\n+CGNSINF: 1,1,%*d,%lf,%lf,%*s,%*s", &modem_latitude, &modem_longitude) == 2) {  
                                ESP_LOGI(TAG, "latitude:%lf, longitude:%lf\n", modem_latitude,modem_longitude);//打印获取的经纬度
                            } else {  
                                ESP_LOGE(TAG, "modem_loc err\n");  
                            }
                        }

                    }
                }
            #else
                uart_write_bytes(uart_init_4g_port, (const char *) "AT+CIPGSMLOC=1,1\r\n", 18);//
                memset(uart_recv_data, 0, 500); len = uart_read_bytes(uart_init_4g_port, uart_recv_data, 500, 100 / portTICK_RATE_MS);
                if (len>0) ESP_LOGI(TAG, "uart_recv_data:%s\n", uart_recv_data);//打印模组返回的数据
                if (len>0)
                {//+CIPGSMLOC: 0,22.7531414,108.3727417,2024/12/02,01:16:48
                    if (sscanf(uart_recv_data, "AT+CIPGSMLOC=1,1\r\n\r\n+CIPGSMLOC: %*d,%lf,%lf,%*s,%*s", &modem_latitude, &modem_longitude) == 2) {  
                        ESP_LOGI(TAG, "latitude:%lf, longitude:%lf\n", modem_latitude,modem_longitude);//打印获取的经纬度
                    } else {  
                        ESP_LOGE(TAG, "modem_loc err\n");  
                    }
                }
            #endif
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


/*****拍照上传到http************************************************************************************************************************************/
esp_err_t _http_event_handler(esp_http_client_event_t *event)
{
    switch (event->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", event->header_key, event->header_value);
        break;
    case HTTP_EVENT_ON_DATA://接收到服务器数据

        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", event->data_len);

         /*把数据发送到队列*/
        queue_data_send(event->data, event->data_len, data_http);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}
TaskHandle_t TaskHandle_http_camera_task;

static void http_camera_task(void *pvParameters)
{
    int len;
    int errCnt=0;int okCnt=0;int err=0;
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t *_jpg_buf;

    uint8_t efuse_mac[8];
    memset(efuse_mac,0,8);

    char data_buff[70]="";

    //http
    esp_http_client_config_t config = {
        .url = "http://192.168.0.2:8081/PHP/audio_camera1.php",//或者https://mnifdv.cn/PHP/audio_camera1.php
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
        .buffer_size_tx = 1460*17,
    };

    #define  define_body_buff_len (1024*1000)
    char *body_buff = (char *)heap_caps_malloc(define_body_buff_len, MALLOC_CAP_SPIRAM);

    /*SD卡*/
    char sdcard_image_name[50];
    FILE *f;
    int image_count=0;
    esp_err_t sdcard_ret = ESP_OK;
    
    uint32_t NotifyValue;//获取任务通知


    #define camera_power_down_enable 1  //使能每次拍照重新给摄像头上电 (500W摄像头长时间工作会发热)

    #if !camera_power_down_enable

        gpio_set_level(gpio_camera_power_pin, 0);//给SD卡和摄像头上电

        gpio_set_level(46, 1);//打开补光灯
        vTaskDelay(pdMS_TO_TICKS(1000));

        //初始化SD卡
        sdcard_ret = sdcard_init();
        if (sdcard_ret != ESP_OK){
            ESP_LOGE(TAG, "Camera SD Card Failed");
        }

        //初始化摄像头
        errCnt=0;
        esp_camera_deinit();
        res = gy_camera_init(&camera_config_init);
        while (res != ESP_OK)
        {
            res = gy_camera_init(&camera_config_init);
            ESP_LOGE(TAG, "Camera Init Failed");
            vTaskDelay(pdMS_TO_TICKS(500));
            errCnt ++;
            if (errCnt>=3)
            {
                #if 1
                    errCnt=0;
                    sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                #else
                    esp_restart();//重启
                #endif
            }
        }
        /*配置摄像垂直或水平镜像*/
        sensor_t *sensor = esp_camera_sensor_get();
        if (sensor->id.PID == 0x5640)
        {
            sensor->set_vflip(sensor,1);//设置垂直翻转(0=关闭, 1=开启)
            // sensor->set_hmirror(sensor, 0); // 水平镜像 (0=关闭, 1=开启)   
        }
        else if (sensor->id.PID == 0x3660)
        {
            // sensor->set_vflip(sensor,1);//设置垂直翻转(0=关闭, 1=开启)
            // sensor->set_hmirror(sensor, 0); // 水平镜像 (0=关闭, 1=开启)   
        }


        #if amera_focus_enable //是否支持自动对焦
            char camera_focus_err=0;
            if ( (res=gy_camera_focus_init(sensor)) != 0) {
                ESP_LOGE(TAG, "camera_focus_init fail, or not supported");
                camera_focus_err=1;
            }
            else
            {
                ESP_LOGD(TAG, "camera_focus_init success");
            }
        #endif

    #endif


    while (true)
    {
        #if camera_power_down_enable

            gpio_set_level(gpio_camera_power_pin, 0);//给SD卡和摄像头上电
            gpio_set_level(46, 1);//打开补光灯
            vTaskDelay(pdMS_TO_TICKS(1000));

            //初始化SD卡
            sdcard_ret = sdcard_init();
            if (sdcard_ret != ESP_OK){
                ESP_LOGE(TAG, "Camera SD Card Failed");
            }

            //初始化摄像头
            errCnt=0;
            esp_camera_deinit();
            res = gy_camera_init(&camera_config_init);
            while (res != ESP_OK)
            {
                res = gy_camera_init(&camera_config_init);
                ESP_LOGE(TAG, "Camera Init Failed");
                vTaskDelay(pdMS_TO_TICKS(500));
                errCnt ++;
                if (errCnt>=3)
                {
                    #if 1
                        errCnt=0;

                        sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                    #else
                        esp_restart();//重启
                    #endif
                }
            }
            /*配置摄像垂直或水平镜像*/
            sensor_t *sensor = esp_camera_sensor_get();
            if (sensor->id.PID == 0x5640)
            {
                // sensor->set_vflip(sensor,1);//设置垂直翻转(0=关闭, 1=开启)
                // sensor->set_hmirror(sensor, 0); // 水平镜像 (0=关闭, 1=开启)   
            }
            else if (sensor->id.PID == 0x3660)
            {
                // sensor->set_vflip(sensor,1);//设置垂直翻转(0=关闭, 1=开启)
                // sensor->set_hmirror(sensor, 0); // 水平镜像 (0=关闭, 1=开启)   
            }


            #if amera_focus_enable //是否支持自动对焦
                char camera_focus_err=0;
                if ( (res=gy_camera_focus_init(sensor)) != 0) {
                    ESP_LOGE(TAG, "camera_focus_init fail, or not supported");
                    camera_focus_err=1;
                }
                else
                {
                    ESP_LOGD(TAG, "camera_focus_init success");
                }
            #endif
        #endif


        /*支持自动对焦,启动对焦, 判断是否对焦完成*/
        #if amera_focus_enable //是否支持自动对焦
        if (camera_focus_err==0)
        {
            errCnt=0;okCnt=0;err=0;
            while (true)
            {
                if ( (res=gy_camera_focus_enable(sensor)) != 0) {
                    ESP_LOGE(TAG, "camera_autofocus_mode fail, or not supported");
                    errCnt++;
                    vTaskDelay(pdMS_TO_TICKS(100)); 
                    if (errCnt>=10)
                    {
                        errCnt=0;
                        err=1;
                        break;
                    }
                }
                else{
                    ESP_LOGE(TAG, "camera_autofocus_mode success");
                    break;
                }
            }


            errCnt=0;okCnt=0;
            while (!err)
            {
                res = gy_camera_focus_tatus(sensor);//判断是否对焦完成
                vTaskDelay(pdMS_TO_TICKS(100));    
                if (res == -1) {
                ESP_LOGE(TAG, "camera_get_fwstatus fail");
                } else if (res == FW_STATUS_S_FOCUSED) {
                    ESP_LOGE(TAG, "camera_get_fwstatus focused");
                    break;
                } else if (res == FW_STATUS_S_FOCUSING) {
                    ESP_LOGE(TAG, "camera_get_fwstatus focusing...");
                }
                else{
                    ESP_LOGE(TAG, "camera_get_fwstatus fail, or not supported");
                    break;
                }
                
                errCnt++;
                if (errCnt>=10)
                {
                    errCnt=0;
                    break;
                }
            }
        }
        #endif


        errCnt=0;okCnt=0;
        /*获取图片*/
        while (true && !fb)
        {
            fb = esp_camera_fb_get();//获取图片
            if (!fb){//获取失败
                res = ESP_FAIL;
                vTaskDelay(pdMS_TO_TICKS(10));
                errCnt++;
                ESP_LOGE(TAG, "Camera capture failed:%d",errCnt);
                if (errCnt>=5)//采集了5次都出错了
                {
                    #if 1
                        errCnt=0;

                        sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                    #else
                        esp_restart();//重启
                        break;
                    #endif
                }
            }
            else//获取成功
            {
                okCnt++;
                ESP_LOGE(TAG, "Camera capture okCnt:%d",okCnt);
                if (okCnt<1)//设置采集第几张
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                    esp_camera_fb_return(fb);fb=NULL;
                }
                else
                {
                    gpio_set_level(46, 0);//关闭补光灯
                    break;
                }
            }
        }
        

        /*拍照成功*/
        if (fb )
        {
            ESP_LOGI(TAG, "image size: %zu bytes;width:%d;height:%d", fb->len,fb->width,fb->height);

            
            //等到获取网络时间////////////////////////////////////////////////////
            res = ESP_OK;
            errCnt=0;okCnt=0;
            while (1)
            {
                if((timeinfo.tm_year+1900)>2023)
                {
                    ESP_LOGI(TAG, "time_stamp ok");
                    res = ESP_OK;
                    break;
                }
                else{
                    ESP_LOGI(TAG, "time_stamp err");
                    res = ESP_FAIL;

                    errCnt++;
                    if (errCnt>=10)
                    {
                        sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }//等到获取网络时间////////////////////////////////////////////////////


            
            //等到获取到电池电压////////////////////////////////////////////////////
            res = ESP_OK;
            errCnt=0;okCnt=0;
            while (1)
            {
                if (modem_vbat_value!=0)
                {
                    ESP_LOGI(TAG, "modem_vbat_value ok:%.2f",modem_vbat_value);
                    res = ESP_OK;
                    break;
                }
                else
                {
                    ESP_LOGI(TAG, "modem_vbat_value err");
                    res = ESP_FAIL;

                    errCnt++;
                    if (errCnt>=10)
                    {
                        sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }//等到获取到电池电压////////////////////////////////////////////////////


            //如果获取的图片格式不是jpg转成jpg////////////////////////////////////////////////////
            if(res == ESP_OK)
            {
                if (fb->format != PIXFORMAT_JPEG)
                {
                    bool jpeg_converted = frame2jpg(fb, 50, &_jpg_buf, &_jpg_buf_len);
                    if (!jpeg_converted)
                    {
                        ESP_LOGE(TAG, "JPEG compression failed");
                        esp_camera_fb_return(fb);fb=NULL;
                        res = ESP_FAIL;
                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }
                }
                else
                {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }//如果获取的图片格式不是jpg转成jpg////////////////////////////////////////////////////

            

            if(res == ESP_OK)
            {
                //把图片存储到SD卡/////////////////////////////////////////////////////////////////////////////////
                if (sdcard_ret == ESP_OK)
                {
                    memset(sdcard_image_name,0,sizeof(sdcard_image_name));
                    sniprintf(sdcard_image_name,sizeof(sdcard_image_name),"%s/%d%s",MOUNT_POINT,image_count,".jpg");

                    ESP_LOGI(TAG, "Opening file %s", sdcard_image_name);
                    FILE *f = fopen(sdcard_image_name, "w");
                    if (f == NULL) {
                        ESP_LOGE(TAG, "Failed to open file for writing");
                    }
                    else{
                        fwrite(fb->buf, fb->len, 1,f);
                        fclose(f);
                        ESP_LOGI(TAG, "File %s OK",sdcard_image_name);

                        image_count++;                    
                    }
                }//把图片存储到SD卡/////////////////////////////////////////////////////////////////////////////////
                

                //把图片通过HTTP发送到服务器/////////////////////////////////////////////////////////////////////////////////
                errCnt=0;okCnt=0;
                while (true)
                {
                    esp_http_client_handle_t esp_client = esp_http_client_init(&config);//初始化

                    //初始化form_data,开始填充数据            数据缓存     缓存数组的长度       打印错误日志
                    gy_http_post_form_data_start(esp_client, body_buff, define_body_buff_len, 1);

                    //上报模组的IMEI
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%s",modem_imei);//modem_imei 是初始化4G模组是底层获取的
                    if (strlen(modem_imei)==0)//使用wifi测试时,这个是空
                    {
                        esp_read_mac(efuse_mac,ESP_MAC_ETH);
                        sprintf(data_buff, "%02X%02X%02X%02X%02X%02X", efuse_mac[0],efuse_mac[1],efuse_mac[2],efuse_mac[3],efuse_mac[4],efuse_mac[5]);
                    }
                    //设置数据                                数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "client_id", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报设置的正常休眠时间(分钟)
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%lld",sleep_time_ok);//modem_sn 是初始化4G模组是底层获取的
                    //设置数据                                 数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "sleep_time_ok", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报设置的错误休眠时间(分钟)
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%lld",sleep_time_err);//modem_sn 是初始化4G模组是底层获取的
                    //设置数据                                 数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "sleep_time_err", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报模组的SN
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%s",modem_sn);//modem_sn 是初始化4G模组是底层获取的
                    //设置数据                             数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "sn", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报模组所安装SIM卡的iccid
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%s",modem_iccid);//modem_iccid 是初始化4G模组是底层获取的
                    //设置数据                               数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "iccid", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报模组的所安装SIM卡的imsi
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%s",modem_imsi);//modem_imsi 是初始化4G模组是底层获取的
                    //设置数据                              数据name                 数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "imsi", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报时间
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff,sizeof(data_buff),"%d-%02d-%02d %02d:%02d:%02d",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
                    //设置数据                             数据name              数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "time", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报电池电压
                    memset(data_buff,0,sizeof(data_buff));
                    snprintf(data_buff, sizeof(data_buff), "%.2f",modem_vbat_value);
                    //设置数据                              数据name              数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "vbat", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报信号强度
                    memset(data_buff,0,sizeof(data_buff));
                    // modem_rssi = modem_rssi*2-113;//modem_imei是初始化4G模组是底层获取的
                    snprintf(data_buff,sizeof(data_buff),"%d",modem_rssi);
                    //设置数据                             数据name              数据        数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "rssi", NULL, NULL, data_buff, strlen(data_buff), 1);

                    //上报文件(图片)                        文件name  详细文件路径名称    上传的是图片         数据      数据长度         打印错误日志
                    gy_http_post_form_data_body(esp_client, "files",     "files",       "image/jpg", (char *)_jpg_buf, _jpg_buf_len, 1);

                    //停止form_data,填充数据完成
                    gy_http_post_form_data_stop(esp_client, 1);

                    res = esp_http_client_perform(esp_client);//发送post数据

                    if (res == ESP_OK)//http发送OK
                    {
                        ESP_LOGI(TAG, "esp_http_client_get_status_code=%d",esp_http_client_get_status_code(esp_client));
                        ESP_ERROR_CHECK(esp_http_client_cleanup(esp_client));//清理

                        break;
                    }
                    else//发送失败
                    {
                        ESP_LOGI(TAG, "esp_err: %s", esp_err_to_name(res));
                        ESP_ERROR_CHECK(esp_http_client_cleanup(esp_client));//清理
                        
                        errCnt++;
                        if (errCnt>=3)
                        {
                            // errCnt=0;
                            break;
                        }
                        vTaskDelay(pdMS_TO_TICKS(2000));//延时一会再次尝试
                    }
                }//把图片通过HTTP发送到服务器/////////////////////////////////////////////////////////////////////////////////

                
                /*释放图片内存*/
                if (fb->format != PIXFORMAT_JPEG)
                {
                    free(_jpg_buf);
                }
                esp_camera_fb_return(fb);fb=NULL;
                /*释放图片内存*/
                
                
                #if camera_power_down_enable
                    gpio_set_level(gpio_camera_power_pin, 1);//给SD卡和摄像头断电
                #endif


                if (errCnt>=3)//上传失败
                {
                    sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
                }

                vTaskDelay(pdMS_TO_TICKS(10000));//延时
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}



/***获取网络时间***************************************************************************************************************************************************************/
void time_sync_notification_cb(struct timeval *tv)//通知时间同步事件
{
    ESP_LOGI(TAG, "time_sync_notification_cb");
}
//初始化SNTP相关函数
const char *sntp_servername[] = {"ntp.aliyun.com","time.tencentcloud.com","time.hicloud.com","ntp.aliyun.com","time1.cloud.tencent.com",};
static void init_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    for (size_t i = 0; i < SNTP_MAX_SERVERS; i++)
    {
        sntp_setservername(i, sntp_servername[i]);
    }
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);//设置时间同步通知事件
    // ntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);//设置SNTP时间同步模式：平滑模式。
    sntp_init();
}
static void sntp_task(void *pvParameters)
{
    int retry = 0;

    while (1)
    {
        init_sntp();
        retry=0;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 100) //等待时间同步完成
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        //设置时区
        setenv("TZ", "CST-8", 1);
        tzset();

        /*获取网络时间*/
        time_stamp = time(&time1);//返回的是时间戳
        // gmtime_r(&time1, &timeinfo);//获取的是格林时间
        localtime_r(&time1, &timeinfo);//获取本地时间

        ESP_LOGI(TAG, "time_stamp:%ld y:%d,m:%d,d:%d %d:%d:%d",time_stamp,timeinfo.tm_year+1900,timeinfo.tm_mon+1, timeinfo.tm_mday,timeinfo.tm_hour, timeinfo.tm_min,timeinfo.tm_sec);

        if((timeinfo.tm_year+1900)>2023)//获取到网络时间
        {
            sntp_stop();
            while(1)
            {
                /*获取网络时间*/
                time_stamp = time(&time1);//返回的是时间戳
                // gmtime_r(&time1, &timeinfo);//获取的是格林时间
                localtime_r(&time1, &timeinfo);//获取本地时间
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        else//上面的时间同步超时了
        {
            sntp_stop();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
/***获取网络时间***/


/***4G初始化回调******************************************************************************************************************************************************************/
static void esp_event_handler_4g(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
    static char modem_timeout_cnt=0;
    if (event_base == MODEM_BOARD_EVENT)
    {
        on_modem_event_id = event_id;
        if (event_id == MODEM_EVENT_SIMCARD_DISCONN)//检测sim卡错误
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_SIMCARD_DISCONN...................11111111111111");
        }
        else if (event_id == MODEM_EVENT_SIMCARD_CONN)
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_SIMCARD_CONN......................222222222222222222222");
        }
        else if (event_id == MODEM_EVENT_DTE_DISCONN)//USB disconnected
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_DTE_DISCONN......................3333333333333333");
        }
        else if (event_id == MODEM_EVENT_DTE_CONN)//USB connected
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_DTE_CONN......................44444444444444");
        }
        else if (event_id == MODEM_EVENT_DTE_RESTART)//Hardware restart
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_DTE_RESTART......................55555555555");
        }
        else if (event_id == MODEM_EVENT_DTE_RESTART_DONE)//Hardware restart done
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_DTE_RESTART_DONE......................6666666666666666");
        }
        else if (event_id == MODEM_EVENT_NET_CONN)//Network connected
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_NET_CONN......................77777777777777");
            modem_timeout_cnt=0;
        }
        else if (event_id == MODEM_EVENT_NET_DISCONN)//Network disconnected
        {
            modem_timeout_cnt++;
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_NET_DISCONN:%d",modem_timeout_cnt);
            if (modem_timeout_cnt>=3)
            {
                modem_timeout_cnt=0;

                sleep_time_gpio_wakeup(sleep_time_err*1000*1000, sleep_gpio_pin, sleep_gpio_wakeup_type);
            }
        }
        else if (event_id == MODEM_EVENT_WIFI_STA_CONN)
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_WIFI_STA_CONN......................999999999999999999");
        }
        else if (event_id == MODEM_EVENT_WIFI_STA_DISCONN)
        {
            ESP_LOGI(TAG, "on_modem_event MODEM_EVENT_WIFI_STA_DISCONN......................000000000000000");
        }
    }
}
/***4G初始化回调***/



/*OTA*************************************************************************************************************************************************************************/
#define ENABLE_OTA  0  //是否打开OTA功能 使用说明看这个: 基础例程- http OTA升级程序


#if ENABLE_OTA 

struct_ota_task_t struct_ota_task={0};

#define HASH_LEN 32
size_t ota_data_len=0;
char ota_callback(struct_ota_state ota_state, char code, char *data, size_t data_len)
{
    if(ota_state == enum_ota_start)
    {
        ota_data_len=0;
        ESP_LOGI(TAG, "ota_state == enum_ota_start");
    }
    else if(ota_state == enum_ota_err_http_read)//异常断开
    {
        ESP_LOGE(TAG, "ota_state == enum_ota_err_http_read");
    }
    else if(ota_state == enum_ota_firmware_version_same)//固件版本相同
    {
        return 0;//返回0:不下载;  返回1:下载
    }
    else if(ota_state == enum_ota_write)//正在循环下载和往flash写数据
    {//data:写数据; data_len:数据长度
        ota_data_len = ota_data_len + data_len;
        ESP_LOGI(TAG, "ota_data_len %d", ota_data_len);//打印已经写入的数据长度
    }
    return 0;
}


/*把此函数放到认为程序执行没有问题的地方执行一次*/
/*如果没有执行  esp_ota_mark_app_valid_cancel_rollback ,设备重启以后会回滚*/
void ota_check(void)
{
    esp_ota_img_states_t ota_state;
    if (gy_ota_get_state_run(&ota_state) == ESP_OK)//是运行的新程序
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)//还没有设置验证的标志位
        {
            ESP_LOGI(TAG, "ota_state == ESP_OTA_IMG_PENDING_VERIFY");
            esp_ota_mark_app_valid_cancel_rollback(); //设置该固件可以用; 把固件状态改为已验证状态
            //esp_ota_mark_app_invalid_rollback_and_reboot();//设置该固件无效,程序里面会重启并执行回滚
        }
        else if (ota_state == ESP_OTA_IMG_VALID)//固件已验证
        {
            ESP_LOGI(TAG, "ota_state == ESP_OTA_IMG_VALID");
        }
        else if (ota_state == ESP_OTA_IMG_INVALID)//固件无效，可能回滚
        {
            ESP_LOGI(TAG, "ota_state == ESP_OTA_IMG_INVALID");
        }
    }
    else
    {
        ESP_LOGE(TAG, "audio_camera_ota_get_state_run");//无效/为空
    }
}


static void printf_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}
/******************************************************************************************************************OTA*/
#endif



/*获取保存的参数*/
void nvs_init(void)
{
    int32_t int32_value=0;
    esp_err_t err = nvs_flash_init();

    /*打开*/     //操作的表格名字 //以读写模式打开
    err = nvs_open("storage", NVS_READWRITE, &my_nvs_handle);

    err = nvs_get_i32(my_nvs_handle, "sleep_time_ok", &int32_value);//休眠时间
    if (err==ESP_OK && int32_value>0)
    {
        sleep_time_ok = int32_value;
        ESP_LOGI(TAG, "sleep_time_ok=%lld",sleep_time_ok);
    }

    err = nvs_get_i32(my_nvs_handle, "sleep_time_err", &int32_value);//执行错误休眠时间
    if (err==ESP_OK && int32_value>0)
    {
        sleep_time_err = int32_value;
        ESP_LOGI(TAG, "sleep_time_err=%lld",sleep_time_err);
    }
    nvs_close(my_nvs_handle);
}



void demo_test(void)
{
    int errCnt=0;int okCnt=0;int err=0;

    nvs_init();

    //获取唤醒源
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_EXT0"); break;//外部中断0唤醒 RTC IO  GPIO0 - GPIO21
        case ESP_SLEEP_WAKEUP_EXT1 : ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_EXT1"); break;//外部中断1唤醒 RTC IO组合式
        case ESP_SLEEP_WAKEUP_TIMER : ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_TIMER"); break;//定时器唤醒
        case ESP_SLEEP_WAKEUP_TOUCHPAD : ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_TOUCHPAD"); break;//触摸引脚唤醒
        case ESP_SLEEP_WAKEUP_ULP : ESP_LOGI(TAG, "ESP_SLEEP_WAKEUP_ULP"); break;//ULP协处理器唤醒
        default : ESP_LOGI(TAG, "wakeup_reason:%d\n",wakeup_reason); break;
    }
    
    ESP_LOGI(TAG, "esp_reset_reason=%d",esp_reset_reason());//获取重启原因
    
    #if ENABLE_OTA 
    char version[32];
    uint8_t sha_256[HASH_LEN] = { 0 };

    gy_ota_get_sha256_table(sha_256);
    printf_sha256(sha_256, "SHA-256 for the partition table: ");

    gy_ota_get_sha256_boot(sha_256);
    printf_sha256(sha_256, "SHA-256 for bootloader: ");

    gy_ota_get_sha256_run(sha_256);
    printf_sha256(sha_256, "SHA-256 for current firmware: ");


    struct_ota_task.task_priority = 7;
    struct_ota_task.task_stack = 1024*12;
    struct_ota_task.http_client_config.url = "http://mnifdv.cn/ota/hardware/audioCamera/A1/audioCamera.bin";
    struct_ota_task.http_client_config.keep_alive_enable = true;
    struct_ota_task.ota_callback = ota_callback;
    struct_ota_task.http_client_config.buffer_size = 17*1024;

    gy_ota_get_version_running(version, 32);//获取当前运行的固件版本
    ESP_LOGI(TAG, "running firmware version: %s", version);
    #endif

    wifi_init();//WiFi初始化
    errCnt=0;okCnt=0;
    while (1)
    {
        if (wifi_struct.sta_state==2)//连接上路由器
        {
            ESP_LOGI(TAG, "connect wifi success");
            //连接路由器分配的IP地址
            ESP_LOGI(TAG, "got ip:%d.%d.%d.%d",
            (((wifi_struct.event->ip_info.ip.addr)>>24)&0xff),
            (((wifi_struct.event->ip_info.ip.addr)>>16)&0xff),
            (((wifi_struct.event->ip_info.ip.addr)>>8)&0xff),
            (((wifi_struct.event->ip_info.ip.addr)>>0)&0xff)
            );
            break;
        }
        else
        {
            errCnt++;
            if (errCnt>=100)//超过10S还没连接上路由器
            {
                errCnt=0;
                ESP_LOGE(TAG, "connect wifi failed");
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (wifi_struct.sta_state!=2)//没连接上路由器
    {
        //4G module init
        gy_4g_init(esp_event_handler_4g);//初始化4G模块(阻塞的,只有连接上网络才会往下执行)
    }
    
    #if ENABLE_OTA 
        gy_ota_task_create(&struct_ota_task);//执行OTA升级(在执行过程中下载失败会自动删除这个任务)
        while (struct_ota_task.runing==1)//如果程序处于执行更新, 不往下执行
        {
            vTaskDelay(pdMS_TO_TICKS(5000));
            ESP_LOGI(TAG, "ota....");
        }
    #endif
    
    //队列传输数据通用模板
    queue_data_init(configMAX_PRIORITIES-1, 1024*8);
    
    //sntp
    xTaskCreate(&sntp_task, "sntp_task", 1024*7, NULL, 11, NULL);

    //配置和4G通信的串口,通过AT指令获取一些信息
    xTaskCreate(&uart_init_4g_task, "uart_init_4g_task", 1024*7, NULL, 12, NULL);

    //摄像头采集图片,并发送到服务器
    xTaskCreate(&http_camera_task, "http_test_task", 1024*16, NULL, 10, &TaskHandle_http_camera_task);

    #if ENABLE_OTA 
        vTaskDelay(pdMS_TO_TICKS(5000));//这里的意思是,等待了5S程序没重启,认为程序没问题了;
        ota_check();/*把此函数放到认为程序执行没有问题的地方执行一次*/
    #endif
}