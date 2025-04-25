#pragma once


typedef enum
{
    MASTER = 0,
    SLAVE,
} DEVICE_TYPE;

typedef enum
{
    DATA_HEAD     = 1,

    // 主命令
    SEARCH_DEVICE = 2,
    REPORT        = 3,   // 从机给主机上报，则为将从机数据层层向上发送，直到主机收到数据
    NOTICE        = 4,
    BROADCAST     = 5,   // 广播消息，表示当前消息所有设备都要出来
    WRITE_CMD     = 6,   // 向指定设备发送命令
    WRITE_DATA    = 7,   // 向指定设备发送数据，例如灯的颜色，预设模式，哪一个灯亮什么颜色等数据，需要包含设备ID和进一步的命令
    WRITE_TEST    = 8,

    // 副命令
    SET_MODE        = 9,
    SET_LINE_COLOR  = 10,
    SET_FULL_COLOR  = 11,
    SET_PIXEL_COLOR = 12,
    SET_BRIGHTNESS  = 13,
    SET_LOOP_COLOR  = 14,

} UART_MSG_TYPE;

DEVICE_TYPE get_device_type();

void set_device_type(DEVICE_TYPE type);

int get_device_index();

void set_device_index(int index);

int get_slave_count();

void uart_manager_init();

void uart_seach_device();

void uart_report_device();

void uart_set_slave_mode(unsigned char mode, char target_deivce_index, char rgb_ch);

void uart_set_slave_full_color(unsigned short color[3], char target_deivce_index, char rgb_ch);

void uart_set_slave_line_color(unsigned char *color_buf, unsigned short len, char target_deivce_index, char rgb_ch);

void uart_set_slave_pixel_color(unsigned char led_index, unsigned short color[3], char target_deivce_index, char rgb_ch);

void uart_set_slave_brightness(char value, char target_deivce_index, char rgb_ch);

void uart_set_slave_loop_color(unsigned char *data, unsigned short len, char target_deivce_index, char rgb_ch);
