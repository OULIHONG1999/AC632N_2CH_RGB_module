#include "rgb_cmd.h"
#include "system/includes.h"
#include "uart_device_manager.h"
#include "broad_led.h"
#include "timer.h"
#include "device_main.h"
#include "uart_cmd.h"

// 此文件中，需要管理连接各个部分的状态，以及处理各个部分的消息。

// 1. 搜索设备，连接设备，断开连接，断开所有连接。

// 2. 管理本机设备上的信息

// 3. 管理连接的设备的信息

void init_uart_io()
{
    gpio_set_direction(MASTER_TX_IO, 0);
    gpio_set_pull_down(MASTER_TX_IO, 0);
    gpio_set_pull_up(MASTER_TX_IO, 1);

    gpio_set_direction(MASTER_RX_IO, 0);
    gpio_set_pull_down(MASTER_RX_IO, 0);
    gpio_set_pull_up(MASTER_RX_IO, 1);
}


void device_main()
{
    init_uart_io();


    // 初始化串口模块
    uart_manager_init();

    // 初始化led io
    init_led_io();
    // 添加定时器反转io
    sys_hi_timer_add(NULL, blink_led_timer, 200);

    // 测试rgb
    init_rgb_ledc();
    // sys_hi_timer_add(NULL, show_rgb_timer, 10);

}
