#define wifi_c_
#include "wifi.h"

#if(1)
#define debug_printf ESP_LOGI
#else
#define debug_printf(...)
#endif

#define TAG  "wifi"


parame_config_wifi_t parame_config_wifi;

wifi_struct_t wifi_struct;

EventGroupHandle_t EventGroupHandleWiFiEvent;

#define EventBitsWiFiStaConnected BIT0

// #region **************************************配置WiFi**********************************************
/*重新连接热点*/
void WIFI_EVENT_STA_DISCONNECTED_FUN(void)
{   
    //事件标志组
    xEventGroupClearBits(EventGroupHandleWiFiEvent, EventBitsWiFiStaConnected);
    esp_wifi_connect();//连接热点
    debug_printf(TAG,"connect to the AP fail");
    wifi_struct.sta_state=0;
}

/*有设备连接上ESP32的热点*/
void WIFI_EVENT_AP_STACONNECTED_FUN( void* event_data  )
{
    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    /*打印连接设备的MAC地址*/
    debug_printf(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
}

// #endregion **************************************配置WiFi**********************************************

/*有设备断开和ESP32的热点*/
void WIFI_EVENT_AP_STADISCONNECTED_FUN( void* event_data  )
{
    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    /*打印断开设备的MAC地址*/
    debug_printf(TAG, "station "MACSTR" leave, AID=%d",MAC2STR(event->mac), event->aid);
}

/*连接上路由器(获取到了分配的IP地址)*/
void IP_EVENT_STA_GOT_IP_FUN( void* event_data )
{
    wifi_struct.event = (ip_event_got_ip_t*) event_data;

    debug_printf(TAG, "got ip:" IPSTR, IP2STR(&wifi_struct.event->ip_info.ip));
    wifi_struct.sta_state=2;
    //事件标志组
    xEventGroupSetBits(EventGroupHandleWiFiEvent, EventBitsWiFiStaConnected);
}


static esp_err_t wifi_set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE))
    {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}
 
char wifi_sta_ip[36];       //设置静态IP
char wifi_sta_netmask[36];  //子网掩码
char wifi_sta_gw[36];       //网关地址

static void wifi_set_static_ip(esp_netif_t *netif)
{
    if (strlen(wifi_sta_ip)>1)
    {
        debug_printf(TAG, "wifi_set_static_ip");
 
        if (esp_netif_dhcpc_stop(netif) != ESP_OK)
        {
            debug_printf(TAG, "Failed to stop dhcp client");
            return;
        }
        esp_netif_ip_info_t ip;
        memset(&ip, 0, sizeof(esp_netif_ip_info_t));
        ip.ip.addr = ipaddr_addr(wifi_sta_ip);
        ip.netmask.addr = ipaddr_addr(wifi_sta_netmask);
        ip.gw.addr = ipaddr_addr(wifi_sta_gw);
        if (esp_netif_set_ip_info(netif, &ip) != ESP_OK)
        {
            debug_printf(TAG, "Failed to set ip info");
            return;
        }
        //ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", EXAMPLE_STATIC_IP_ADDR, EXAMPLE_STATIC_NETMASK_ADDR, EXAMPLE_STATIC_GW_ADDR);
        //ESP_ERROR_CHECK(wifi_set_dns_server(netif, ipaddr_addr(EXAMPLE_MAIN_DNS_SERVER), ESP_NETIF_DNS_MAIN));
        //ESP_ERROR_CHECK(wifi_set_dns_server(netif, ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));   
    }
    else
    {

    }
}



/*WiFi事件回调*/
void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    /* 事件剥离 */
    if( event_base == WIFI_EVENT )
    {
        switch ( event_id )
        {
            case WIFI_EVENT_STA_START:          esp_wifi_connect();                  break; // STA START
            case WIFI_EVENT_STA_STOP:           debug_printf(TAG,"WIFI_EVENT_STA_STOP"); break; // STA STOP 
            case WIFI_EVENT_STA_CONNECTED:      wifi_set_static_ip(arg); wifi_struct.sta_state=1; break;
            case WIFI_EVENT_STA_DISCONNECTED:   WIFI_EVENT_STA_DISCONNECTED_FUN();   break; //和路由器断开
            case WIFI_EVENT_AP_START:           debug_printf(TAG,"WIFI_EVENT_AP_START"); break; // AP  START 
            case WIFI_EVENT_AP_STOP:            debug_printf(TAG,"WIFI_EVENT_AP_STOP");  break; // AP  STOP
            case WIFI_EVENT_AP_STACONNECTED:    WIFI_EVENT_AP_STACONNECTED_FUN( event_data );    break; //有设备连接上ESP32的热点
            case WIFI_EVENT_AP_STADISCONNECTED: WIFI_EVENT_AP_STADISCONNECTED_FUN(event_data );  break; //有设备断开和ESP32的热点                    
            default:  break;
        }
    }
    else if( event_base == IP_EVENT )  // 路由事件ID 组
    {
        switch ( event_id )
        {
            case IP_EVENT_STA_GOT_IP:        IP_EVENT_STA_GOT_IP_FUN(event_data);       break; //获取到指定IP
            case IP_EVENT_STA_LOST_IP:       debug_printf(TAG,"IP_EVENT_STA_LOST_IP");      break;
            case IP_EVENT_AP_STAIPASSIGNED:  debug_printf(TAG,"IP_EVENT_AP_STAIPASSIGNED"); break;
            default:  break;
        }
    }
}



EventGroupHandle_t EventGroupHandleWiFiEvent;


void wifi_init_softap_sta(parame_config_wifi_t *wifi)
{
    if (wifi->mode>0)
    {
        esp_event_handler_instance_t instance_any_id = {0};  //处理ID 实例句柄
        esp_event_handler_instance_t instance_got_ip = {0};  //处理IP 实例句柄

        esp_netif_t *ap_netif  = esp_netif_create_default_wifi_ap();//创建有 TCP/IP 堆栈的默认网络接口实例绑定AP。
        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();//创建有 TCP/IP 堆栈的默认网络接口实例绑定STA。

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));//创建 Wi-Fi 驱动程序任务，并初始化 Wi-Fi 驱动程序。

        // /*注册系统事件回调函数*/
        // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,           //WiFi事件函数
        //                                                     ESP_EVENT_ANY_ID,     //事件ID
        //                                                     &wifi_event_handler,  //回调函数
        //                                                     sta_netif,
        //                                                     &instance_any_id));
        // /*注册系统事件回调函数*/
        // ESP_ERROR_CHECK( esp_event_handler_instance_register(IP_EVENT,
        //                                                     IP_EVENT_STA_GOT_IP,
        //                                                     &wifi_event_handler,
        //                                                     sta_netif,
        //                                                     &instance_got_ip) );


        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
      
        ESP_ERROR_CHECK(esp_wifi_set_mode(wifi->mode));//设置AP+STA模式
        esp_wifi_set_storage(WIFI_STORAGE_FLASH);
        ESP_ERROR_CHECK(esp_wifi_start());//启动


        memset(wifi_sta_ip,0,sizeof(wifi_sta_ip));//设置静态IP
        memcpy(wifi_sta_ip, wifi->sta_ip, strlen(wifi->sta_ip));

        memset(wifi_sta_netmask,0,sizeof(wifi_sta_netmask));//子网掩码
        memcpy(wifi_sta_netmask, wifi->sta_netmask, strlen(wifi->sta_netmask));

        memset(wifi_sta_gw,0,sizeof(wifi_sta_gw));//网关地址
        memcpy(wifi_sta_gw, wifi->sta_gw, strlen(wifi->sta_gw));


        /*配置连接的热点参数*/
        wifi_config_t wifi_config_sta = {
            .sta = {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,//加密方式
                /*配置pmf,当前最新加密技术*/
                .pmf_cfg = {
                    .capable = true,  //告诉热点这边有能力使用PMF进行加密通信(防止窃听密码)
                    .required = false //告诉热点这边不强制要求使用PMF进行加密通信(防止窃听密码)
                },
            },
        };
        // if (wifi->mode == WIFI_MODE_STA || wifi->mode == WIFI_MODE_APSTA)
        {
            memset(wifi_config_sta.sta.ssid,0,sizeof(wifi_config_sta.sta.ssid));
            memcpy(wifi_config_sta.sta.ssid, wifi->sta_ssid, strlen(wifi->sta_ssid));
            if(strlen(wifi->sta_pass)==0)//没有密码
            {
                wifi_config_sta.sta.threshold.authmode = WIFI_AUTH_OPEN;//加密方式
            }
            else{
                memset(wifi_config_sta.sta.password,0,sizeof(wifi_config_sta.sta.password));
                memcpy(wifi_config_sta.sta.password, wifi->sta_pass, strlen(wifi->sta_pass));
            }
        }
        

        /*配置热点*/
        wifi_config_t wifi_config_ap = {
            .ap = {
                // .ssid = ESP_WIFI_AP_SSID,             
                // .ssid_len = strlen(ESP_WIFI_AP_SSID),
                .channel = wifi->ap_channel,
                // .password = ESP_WIFI_AP_PASS,
                .max_connection = wifi->ap_max_count,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                .ssid_hidden =0,//是否隐藏热点
            },
        };
        

        //     uint8_t efuse_mac[6];
        //     esp_read_mac(efuse_mac,ESP_MAC_ETH);
        //     sprintf(mac,"%02X%02X%02X%02X%02X%02X", efuse_mac[0],efuse_mac[1],efuse_mac[2],efuse_mac[3],efuse_mac[4],efuse_mac[5]);
        // if (wifi->mode == WIFI_MODE_AP || wifi->mode == WIFI_MODE_APSTA)
        {
            memset(wifi_config_ap.ap.ssid,0,sizeof(wifi_config_ap.ap.ssid));
            
            memcpy(wifi_config_ap.ap.ssid, wifi->ap_ssid, strlen(wifi->ap_ssid));


            wifi_config_ap.ap.ssid_len = strlen((char*)wifi_config_ap.ap.ssid);
            
            /*如果密码长度是0,则不设置密码*/
            if (strlen(wifi->ap_pass)==0) {
                wifi_config_ap.ap.authmode = WIFI_AUTH_OPEN;
            }
            else{
                memset(wifi_config_ap.ap.password,0,sizeof(wifi_config_ap.ap.password));
                memcpy(wifi_config_ap.ap.password, wifi->ap_pass, strlen(wifi->ap_pass));
            }
        }

        
        // 设置AP模式配置
        if (wifi->mode == WIFI_MODE_AP || wifi->mode == WIFI_MODE_APSTA)
        {
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP , &wifi_config_ap) );
            esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_BW_HT40);

            if (wifi->ap_4g_wifi_enable)
            {
                gy_4g_wifi_enable(ap_netif);
            }
        }
        
        // 设置STA模式配置
        if (wifi->mode == WIFI_MODE_STA || wifi->mode == WIFI_MODE_APSTA)
        {
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta) );
        }
        
        // ESP_ERROR_CHECK(esp_wifi_start());//启动
        EventGroupHandleWiFiEvent = xEventGroupCreate();
    }
}



void wifi_init(void)
{   //WiFi参数
    uint8_t efuse_mac[8];
    memset(efuse_mac,0,8);
    char mac[20];
    memset(mac,0,20);

    parame_config_wifi.mode = default_wifi_mode;//WIFI_MODE_STA:连接路由器模式;    WIFI_MODE_AP:开热点模式;    WIFI_MODE_APSTA:同时开热点和连接路由器

    /*设置AP模式热点名称*/
    parame_config_wifi.ap_4g_wifi_enable = default_wifi_ap_4g_wifi_enable;//是否打开4G热点,随身wifi功能

    parame_config_wifi.ap_ssid_add_mac = default_wifi_ap_ssid_add_mac;//是否设置无线名称后缀增加模组的MAC地址信息(名称动态唯一); 1:设置 0:不设置

    memset(parame_config_wifi.ap_ssid,0,sizeof(parame_config_wifi.ap_ssid));
    strcpy(parame_config_wifi.ap_ssid, default_wifi_ap_ssid);
    if (parame_config_wifi.ap_ssid_add_mac!=0)
    {
        esp_read_mac(efuse_mac,ESP_MAC_ETH);
        sprintf(mac, "%02X%02X%02X%02X%02X%02X", efuse_mac[0],efuse_mac[1],efuse_mac[2],efuse_mac[3],efuse_mac[4],efuse_mac[5]);
        strcpy(parame_config_wifi.ap_ssid+strlen(default_wifi_ap_ssid), mac);
    }
    debug_printf(TAG, "ap_ssid=%s",parame_config_wifi.ap_ssid);

    /*设置AP模式热点密码*/
    memset(parame_config_wifi.ap_pass,0,sizeof(parame_config_wifi.ap_pass));
    strcpy(parame_config_wifi.ap_pass, default_wifi_ap_pass);

    /*设置AP模式通道和连接数*/
    parame_config_wifi.ap_channel = default_wifi_ap_channel;
    parame_config_wifi.ap_max_count = default_wifi_ap_max_count;

    /*设置连接路由器的名称和密码*/
    memset(parame_config_wifi.sta_ssid,0,sizeof(parame_config_wifi.sta_ssid));
    strcpy(parame_config_wifi.sta_ssid, default_wifi_sta_ssid);
    memset(parame_config_wifi.sta_pass,0,sizeof(parame_config_wifi.sta_pass));
    strcpy(parame_config_wifi.sta_pass, default_wifi_sta_pass);

    /*设置连接路由器后的静态IP*/
    memset(parame_config_wifi.sta_ip,0,sizeof(parame_config_wifi.sta_ip));
    strcpy(parame_config_wifi.sta_ip, default_wifi_sta_ip);//设置静态IP
    memset(parame_config_wifi.sta_netmask,0,sizeof(parame_config_wifi.sta_netmask));
    strcpy(parame_config_wifi.sta_netmask, default_wifi_sta_netmask);//子网掩码
    memset(parame_config_wifi.sta_gw,0,sizeof(parame_config_wifi.sta_gw));
    strcpy(parame_config_wifi.sta_gw, default_wifi_sta_gw);//网关地址


    //配置WiFi
    wifi_init_softap_sta(&parame_config_wifi);
}



