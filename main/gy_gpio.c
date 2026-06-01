#define gy_gpio_c_
#include "gy_gpio.h"

#define TAG "gy_gpio"


//实例1:GPIO输出高低电平(不建议使用这个)
void gy_gpio_test1(void)
{
    //GPIO21              输出模式      下拉              不使用中断    中断函数为空
    gy_gpio_setup(21, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ONLY, GPIO_INTR_MAX, NULL);

    gpio_set_level(21, 0); // 输出低电平
    gpio_set_level(21, 1); // 输出高电平
}


//实例2:GPIO作为输入输出模式(这样子可以检测到输出的状态)
void gy_gpio_test2(void)
{
    //GPIO21              输出/输出模式             上拉          不使用中断    中断函数为空
    gy_gpio_setup(21, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_ONLY, GPIO_INTR_MAX, NULL);

    gpio_set_level(21, 0); // 输出低电平
    gpio_set_level(21, 1); // 输出高电平
    
    if (gpio_get_level(21)==0)//获取电平状态
    {
        //低电平
    }
    else
    {
        //高电平
    }
}



//实例3:GPIO仅作为输入
void gy_gpio_test3(void)
{
    //GPIO21             输入模式             上拉          不使用中断    中断函数为空
    gy_gpio_setup(21, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_MAX, NULL);

    if (gpio_get_level(21)==0)//获取电平状态
    {
        //低电平
    }
    else
    {
        //高电平
    }
}



//实例4:GPIO作为中断
void gpio_isr_handler(void* arg)
{
    
}
void gy_gpio_test4(void)
{
    //GPIO21              输出/输出模式        上拉          下降沿触发          中断函数
    gy_gpio_setup(21, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY, GPIO_INTR_NEGEDGE, gpio_isr_handler);
}



//实例5: 按键复位例子
uint64_t gpio_key_reset_cnt1=0;
uint64_t gpio_key_reset_cnt2=0;
#define gpio_key_reset_pin 17
void gy_gpio_key_reset(void)
{    
    //配置GPIO
    gy_gpio_setup(gpio_key_reset_pin, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_ONLY, GPIO_INTR_MAX, NULL);

    while(true)
    {
        if (gpio_get_level(gpio_key_reset_pin)==0)
        {
            gpio_key_reset_cnt1++;
        }
        else
        {
            gpio_key_reset_cnt2 = gpio_key_reset_cnt1;
            gpio_key_reset_cnt1=0;
        }

        if (gpio_get_level(gpio_key_reset_pin))
        {
            //主轮训是20ms  按下时间大于60ms 小于1.5S认为是复位
            if (gpio_key_reset_cnt2>(60/20) && gpio_key_reset_cnt2<(1500/20))
            {
                esp_restart();
            }
            else
            {
                gpio_key_reset_cnt2=0;
            }
        }
    }
}


//配置GPIO
void gy_gpio_setup(uint8_t pin, gpio_mode_t mode, gpio_pull_mode_t pull, gpio_int_type_t intr_type, gpio_isr_t isr_handler)
{
    gpio_pad_select_gpio(pin);//选择GPIO口
    
    gpio_set_direction(pin, mode);//输入输出模式

    gpio_set_pull_mode(pin, pull);//上下拉

    if (intr_type != GPIO_INTR_MAX)
    {
        gpio_set_intr_type(pin, intr_type);

        gpio_install_isr_service(0);
        gpio_isr_handler_add(pin, isr_handler, NULL);// 注册中断处理函数
    }
}







