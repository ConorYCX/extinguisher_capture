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
#ifndef gy_gpio_ch_
#define gy_gpio_ch_

#ifndef gy_gpio_c_
#define gy_gpio_cx_ extern
#else
#define gy_gpio_cx_
#endif


#if(1)
#define debug_printf ESP_LOGI
#else
#define debug_printf(...)
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


void gy_gpio_setup(uint8_t pin, gpio_mode_t mode, gpio_pull_mode_t pull, gpio_int_type_t intr_type, gpio_isr_t isr_handler);


#endif


