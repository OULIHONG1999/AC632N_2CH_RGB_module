#include "uart_cmd.h"
#include "stdio.h"
#include "uart_device_manager.h"
#include "rgb_cmd.h"
#include "timer.h"
// 实现设备级联的功能
// 自己既是上位机，也是下位机
// 由上位机发起查询功能，查询级联总线上的所有设备


#define DEVICE_NAME 0x01

// 初始化自己为一个从机设备
static DEVICE_TYPE device_type = SLAVE;

// 保存从机的数量
static int slave_count = 0;
// 在未得到上位机的查询指令之前，自己的设备索引为0
static int device_index = 0;

DEVICE_TYPE get_device_type()
{
    return device_type;
}

void set_device_type(DEVICE_TYPE type)
{
    device_type = type;
}

int get_device_index()
{
    return device_index;
}
void set_device_index(int device_index)
{
    device_index = device_index;
}

int get_slave_count()
{
    return slave_count;
}

/// @brief 收到来自主机的数据
/// @param buf
/// @param len
void received_uart_master_callback(unsigned char *buf, unsigned char len)
{
    // 数据命令格式  包头        消息类型        命令        从机索引       数据长度   数据  CRC 包尾
    //              DATA_HEAD  UART_MSG_TYPE   WRITE_CMD  device_index   deta_len  data  crc END
    printf("received master data: %d\n", len);
    // 打印数据
    put_buf(buf, len);
    // 查询包头
    if (buf[0] != DATA_HEAD)
    {
        printf("error head\n");
        return;
    }

    /*
    收到主机的查询命令
    1、更新自己的index
    2、向最高级主机上报自己的index
    3、向从机发送查询命令
     */
    if (buf[1] == SEARCH_DEVICE)
    {
        printf("get master cmd [SEARCH_DEVICE]\n");
        /*
                            0          1        2          3             4                    4         5     6   7
                数据命令格式 包头       消息类型  命令       本机索引       目标设备索引          数据长度   数据  CRC 包尾
                            DATA_HEAD  REPORT   WRITE_CMD  device_index  target_deivce_index  deta_len  data  crc END
         */
        // 查看主机的设备索引
        char master_index = buf[3];
        // 修改自己的设备索引
        device_index = master_index + 1;
        // 向最高级主机发送上报消息REPORT
        unsigned char report[7] = {DATA_HEAD, REPORT, SEARCH_DEVICE, device_index, DEVICE_NAME, 0xff, 0xff};
        uart_master_send(report, 7);

        unsigned char search_cmd[4] = {DATA_HEAD, SEARCH_DEVICE, SEARCH_DEVICE, device_index};
        // 向从机发送查询命令
        uart_slave_send(search_cmd, 4);
    }
    else if (buf[1] == WRITE_CMD)
    {
        printf("get master cmd [WRITE_CMD]\n");
        printf("traget index %d, local index: %d\n", buf[4], device_index);
        // 获取从机索引，判断是否是发送给本设备的命令
        if (device_index != buf[4])
        {
            // 向从机发送命令
            uart_slave_send(buf, len);
            return;
        }

        unsigned short *rgb_timer_id0 = get_rgb_timer_id(0);
        unsigned short *rgb_timer_id1 = get_rgb_timer_id(1);
        unsigned char mode = buf[2];
        unsigned char rgb_ch = buf[6];
        if (mode != SET_MODE)
        {
            // 将定时器杀掉
            if (rgb_ch == 0)
            {
                if (rgb_timer_id0 != 0)
                {
                    printf("kill timer %d\n", rgb_timer_id0);
                    sys_timer_del(*rgb_timer_id0);
                    *rgb_timer_id0 = 0;
                }
            }
            else if (rgb_ch == 1)
            {
                if (rgb_timer_id1 != 0)
                {
                    printf("kill timer %d\n", rgb_timer_id1);
                    sys_timer_del(*rgb_timer_id1);
                    *rgb_timer_id1 = 0;
                }
            }
        }

        // 查看命令
        unsigned char cmd = buf[2];
        // 判断命令
        switch (cmd)
        {
        case SET_MODE:
            // 获取模式&设置模式
            rgb_set_mode(buf[7], buf[6]);
            break;
        case SET_LINE_COLOR:
            put_buf(&buf[6], buf[5] - 1);
            rgb_set_line_color(&buf[7], buf[5] - 1, buf[6]);
            break;
        case SET_FULL_COLOR:
            //                 color    rgb channel
            rgb_set_full_color(&buf[7], buf[6]);
            break;
        case SET_PIXEL_COLOR:
            rgb_set_pixel_color(buf[7], &buf[8], buf[6]);
            break;
        case SET_BRIGHTNESS:
            rgb_set_brightness(buf[7], buf[6]);
            break;
        case SET_LOOP_COLOR:
            rgb_set_loop_color(&buf[7], buf[5] - 1, buf[6]);
            break;
        default:
            printf("uart_cmd_handle: unknown cmd %d\n", buf[1]);
            break;
        }
    }
    else if (buf[1] == WRITE_TEST)
    {
        printf("get master cmd [WRITE_TEST]\n");
        // 测试命令定义
        // 0    1          2        3        4
        // 包头 WRITE_TEST 测试命令 目标设备  目标RGB通道
        //

        printf("test cmd : %d\n", buf[2]);
        unsigned short color[3] = {0x00};
        switch (buf[2])
        {
        case 0:
            printf("set mode device[%d] rgbch[%d] mode: %X\n", buf[3], buf[4], RGB_MODE_ON);
            uart_set_slave_mode(RGB_MODE_ON, buf[3], buf[4]);
            break;
        case 1:
            color[0] = 0x12;
            color[1] = 0x22;
            color[2] = 0x43;
            printf("set_slave_full_color device[%d] rgbch[%d] color: %X\n",
                   buf[3], buf[4], color[0] << 16 | color[1] << 8 | color[2]);
            uart_set_slave_full_color(color, buf[3], buf[4]);
            break;
        case 2:
            unsigned char rgb[6] = {1, 2, 3, 4, 5, 6};
            printf("set_slave_line_color device[%d] rgbch[%d]\n", buf[3], buf[4]);
            put_buf(rgb, 6);
            uart_set_slave_line_color(rgb, 6, buf[3], buf[4]);
            break;
        case 3:
            color[0] = 0x22;
            color[1] = 0x42;
            color[2] = 0x6a;
            printf("set_slave_pixel_color device[%d] rgbch[%d] color: %X\n",
                   buf[3], buf[4], color[0] << 16 | color[1] << 8 | color[2]);
            uart_set_slave_pixel_color(buf[4], color, buf[3], buf[4]);
            break;
        case 4:
            printf("set brightness device[%d] rgbch[%d] color: %X\n", buf[3], buf[4], color);
            printf("set slave brightness %d\n", buf[4]);
            uart_set_slave_brightness(buf[4], buf[3], buf[4]);
            break;
        default:
            printf("error cmd\n");
            break;
        }
    }
}

/// @brief 收到来自从机的数据
/// @param buf
/// @param len
void received_uart_slave_callback(unsigned char *buf, unsigned char len)
{

    // 数据命令格式  包头        消息类型        命令        从机索引       数据长度   数据  CRC 包尾
    //               DATA_HEAD  UART_MSG_TYPE   WRITE_CMD  device_index   deta_len  data  crc END

    printf("received slave data: %d\n", len);
    // 打印数据
    put_buf(buf, len);

    // 查询包头
    if (buf[0] != DATA_HEAD)
    {
        printf("error head\n");
        return;
    }

    if (buf[1] == REPORT)
    {
        //              0           1         2      3              4         5     6   7
        // 数据命令格式  包头        消息类型   命令   从机索引       数据长度   数据  CRC 包尾
        //              DATA_HEAD   REPORT    CMD    device_index   deta_len  data  crc END

        // 判断自己是不是主机
        if (device_type == MASTER)
        {
            // 如果自己是主机，就处理来自从机的命令
            char receive_device_index = buf[3];
            // 查询从机要做什么
            printf("receive device index: %d\n", receive_device_index);
        }
        else
        {
            // 如果自己不是主机,将原始数据继续上报，直到主机收到数据
            printf("get reeport data\n");
            uart_master_send(buf, len); // 即在当前函数中，需要再次处理这个数据
        }
    }
    else
    {
        printf("get slave cmd erro!\n");
    }
}

// 向最上层主机，报告有新设备连接，让主机发送扫描命令
void report_new_device(unsigned char index)
{
    unsigned char data[4] = {DATA_HEAD, REPORT, SEARCH_DEVICE, device_index};
    uart_master_send(data, 4);
    uart_slave_send(data, 4);
}

void uart_manager_init()
{
    uart_init_master(1000000);
    uart_init_slave(1000000);

    uart_callback_register(received_uart_master_callback, MASTER);
    uart_callback_register(received_uart_slave_callback, SLAVE);

    sys_s_hi_timerout_add(NULL, report_new_device, 1000);
    // sys_hi_timer_add(NULL, report_new_device, 200);
}

void uart_seach_device()
{
    unsigned char cmd[4] = {DATA_HEAD, SEARCH_DEVICE, 0, device_index};
    uart_slave_send(cmd, 4);
}

void uart_report_device()
{
    unsigned char cmd[4] = {DATA_HEAD, REPORT, 0, device_index};
    uart_master_send(cmd, 4);
}

// 数据命令格式  包头        消息类型        命令        从机索引       数据长度   数据  CRC 包尾
//               DATA_HEAD  UART_MSG_TYPE   WRITE_CMD  device_index   deta_len  data  crc END

/// @brief 设置灯带当前的显示模式
/// @param mode
/// @param rgb_ch
void uart_set_slave_mode(unsigned char mode, char target_deivce_index, char rgb_ch)
{
    unsigned char cmd[10] = {DATA_HEAD,
                             WRITE_CMD,
                             SET_MODE,
                             device_index,
                             target_deivce_index,
                             2, // 数据长度
                             rgb_ch,
                             mode,
                             0xff,
                             0xff};
    uart_slave_send(cmd, 10);
}

/// @brief 设置灯带显示所有灯带显示同一种颜色
/// @param color
/// @param rgb_ch
void uart_set_slave_full_color(unsigned short color[3], char target_deivce_index, char rgb_ch)
{
    unsigned char cmd[12] = {DATA_HEAD,
                             WRITE_CMD,
                             SET_FULL_COLOR,
                             device_index,
                             target_deivce_index,
                             3,      // 数据长高度
                             rgb_ch, // 需要设置的led通道
                             color[0],
                             color[1],
                             color[2],
                             0xff,  // RCR
                             0xff}; // 包尾
    uart_slave_send(cmd, 12);
}

/// @brief  设置灯带显示每个灯都显示指定的颜色
/// @param color_buf
/// @param len
/// @param rgb_ch
void uart_set_slave_line_color(unsigned char *color_buf, unsigned short len, char target_deivce_index, char rgb_ch)
{
    const unsigned short cmd_len = 9 + len;
    unsigned char *cmd = calloc(cmd_len, sizeof(unsigned char));
    cmd[0] = DATA_HEAD;
    cmd[1] = WRITE_CMD;
    cmd[2] = SET_LINE_COLOR;
    cmd[3] = device_index;
    cmd[4] = target_deivce_index;
    cmd[5] = len + 1;
    cmd[6] = rgb_ch;
    for (int i = 0; i < len; i++)
    {
        cmd[7 + i] = color_buf[i];
    }
    cmd[cmd_len - 2] = 0xff;
    cmd[cmd_len - 1] = 0xff;
    uart_slave_send(cmd, cmd_len);
    free(cmd);
}

/// @brief 设置灯带中的某一个灯显示某种颜色
/// @param led_index
/// @param color
/// @param rgb_ch
void uart_set_slave_pixel_color(unsigned char led_index, unsigned short color[3], char target_deivce_index, char rgb_ch)
{
    const unsigned short cmd_len = 13;
    unsigned char *cmd = calloc(cmd_len, sizeof(unsigned char));
    cmd[0] = DATA_HEAD;
    cmd[1] = WRITE_CMD;
    cmd[2] = SET_PIXEL_COLOR;
    cmd[3] = device_index;
    cmd[4] = target_deivce_index;
    cmd[5] = 4; // data len
    cmd[6] = rgb_ch;
    cmd[7] = led_index;
    for (char i = 0; i < 3; i++)
    {
        cmd[8 + i] = color[i];
    }
    cmd[11] = 0xff;
    cmd[12] = 0xff;
    uart_slave_send(cmd, cmd_len);
    free(cmd);
}

/// @brief 设置当前灯带显示的亮度
/// @param value
/// @param rgb_ch
void uart_set_slave_brightness(char value, char target_deivce_index, char rgb_ch)
{
    unsigned char cmd[10] = {DATA_HEAD,
                             WRITE_CMD,
                             SET_BRIGHTNESS,
                             device_index,
                             target_deivce_index,
                             2,
                             rgb_ch,
                             value,
                             0xff,
                             0xff};
    uart_slave_send(cmd, 10);
}

/// @brief 一个循环脚本命令，轮询定时显示多长时间，以及显示的颜色，支持每个颜色都自定义
/// @param data
/// @param len
/// @param rgb_ch
void uart_set_slave_loop_color(unsigned char *data, unsigned short len, char target_deivce_index, char rgb_ch)
{
    const unsigned short cmd_len = 9 + len;
    unsigned char *cmd = calloc(cmd_len, sizeof(unsigned char));
    cmd[0] = DATA_HEAD;
    cmd[1] = WRITE_CMD;
    cmd[2] = SET_LOOP_COLOR;
    cmd[3] = device_index; // 本机设备地址
    cmd[4] = target_deivce_index;
    cmd[5] = len + 1;
    cmd[6] = rgb_ch;
    for (int i = 0; i < len; i++)
    {
        cmd[7 + i] = data[i];
    }
    cmd[cmd_len - 2] = 0xff;
    cmd[cmd_len - 1] = 0xff;
    uart_slave_send(cmd, cmd_len);
}