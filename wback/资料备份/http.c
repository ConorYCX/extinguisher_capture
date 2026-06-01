// 结构体成员如下
typedef struct {
    const char                  *url;//HTTP/HTTPS的url地址
    const char                  *host;//域名或ip（此项可不填）
    int                         port;//请求的端口80或443（默认此项可不填）
    const char                  *username;//用户名用于HTTP身份验证（不用不填）
    const char                  *password;//密码用于HTTP身份验证（不用不填）
    esp_http_client_auth_type_t auth_type;//HTTP身份验证类型默认为HTTP_AUTH_TYPE_NONE即无身份验证（默认即可）
    const char                  *path;//HTTP路径默认为/（客户端无需此项）
    const char                  *query;//HTTP查询
    const char                  *cert_pem;//服务器SSL证书（作为HTTPS服务器时）
    size_t                      cert_len;//证书长度
    const char                  *client_cert_pem;//客户端SSL证书（作为客户端，服务器需要验证证书时）
    size_t                      client_cert_len;//证书长度
    const char                  *client_key_pem;//客户端证书密钥
    size_t                      client_key_len;//密钥长度
    const char                  *client_key_password;//密钥密码
    size_t                      client_key_password_len;//密码长度
    const char                  *user_agent;//要与 HTTP 请求一起发送的用户代理字符串
    esp_http_client_method_t    method;//请求模式默认为HTTP_METHOD_GET即GET方法可根据需要进行修改，也可以默认不填请求时直接调用接口
    int                         timeout_ms;//网络超时时间
    bool                        disable_auto_redirect;//禁用HTTP自动重定向
    int                         max_redirection_count;//最大重定向数，如果为零，则使用默认值
    int                         max_authorization_retries;//接收 HTTP 未授权状态代码时的最大连接重试次数，如果为零，则使用默认值。如果 -1 则禁用授权重试
    http_event_handle_cb        event_handler;//HTTP事件回调函数（重要）
    esp_http_client_transport_t transport_type;//HTTP传输类型使用HTTP时填HTTP_TRANSPORT_OVER_TCP，使用HTTPS时填HTTP_TRANSPORT_OVER_SSL
    int                         buffer_size;//HTTP接收缓冲区大小（一般默认即可）
    int                         buffer_size_tx;//HTTP传输缓冲区大小
    void                        *user_data;//HTTP user_data上下文
    bool                        is_async;//设置异步模式，目前仅支持 HTTPS
    bool                        use_global_ca_store;//
    bool                        skip_cert_common_name_check;//跳过服务器证书 CN 字段的任何验证（注意此处有坑）
    esp_err_t (*crt_bundle_attach)(void *conf);//指向esp_crt_bundle_attach的函数指针。允许使用证书捆绑包进行服务器验证，必须在menuconfig中启用
    bool                        keep_alive_enable;//启用保持活动状态超时
    int                         keep_alive_idle;//保持活动空闲时间。默认值为 5（秒）
    int                         keep_alive_interval;//保持活动间隔时间。默认值为 5（秒）
    int                         keep_alive_count;//保持活动状态数据包重试发送计数。默认值为 3 个计数
    struct ifreq                *if_name;//要通过的数据的接口的名称。使用默认界面而不设置
} esp_http_client_config_t;
