#include "system/includes.h"
#include "system/debug.h"
#include "generic/gpio.h"
#include "asm/clock.h"
#include "asm/ledc.h"
#include "rgb_cmd.h"

// *INDENT-OFF*
/*******************************    参考示例 ***********************************/

LEDC_PLATFORM_DATA_BEGIN(user_ledc0_data)
    .index = 1,
   .port = IO_PORTA_00,
   .idle_level = 0,
   .out_inv = 0,
   .bit_inv = 1,
   .t_unit = t_42ns,
   .t1h_cnt = 24,
   .t1l_cnt = 7,
   .t0h_cnt = 7,
   .t0l_cnt = 24,
   .t_rest_cnt = 20000,
   .cbfun = NULL,
   LEDC_PLATFORM_DATA_END();

LEDC_PLATFORM_DATA_BEGIN(user_ledc1_data)
    .index = 0,
   .port = IO_PORTB_05,
   .idle_level = 0,
   .out_inv = 0,
   .bit_inv = 1,
   .t_unit = t_42ns,
   .t1h_cnt = 24,
   .t1l_cnt = 7,
   .t0h_cnt = 7,
   .t0l_cnt = 24,
   .t_rest_cnt = 20000,
   .cbfun = NULL,
   LEDC_PLATFORM_DATA_END();

#define LED_NUM_MAX 35

#define led_send_rgbbuf(ch, buf, num) ledc_send_rgbbuf_isr(ch, buf, num, LED_NUM_MAX-1)

static u8 ledc_test_buf0[3 * LED_NUM_MAX] __attribute__((aligned(4)));
static u8 ledc_test_buf1[3 * LED_NUM_MAX] __attribute__((aligned(4)));

void init_rgb_ledc(void)
{
    printf("************* init ledc test  **************\n");

    ledc_init(&user_ledc0_data);
    ledc_init(&user_ledc1_data);
}

void show_rgb_timer()
{

    static u8 r_val = 0;
    static u8 g_val = 85;
    static u8 b_val = 175;
    static u16 sec_num = LED_NUM_MAX; // 循环发送的次数，用于一条大灯带又分为几条效果一样的小灯带

    r_val += 1;
    g_val += 1;
    b_val += 1;

    ledc_rgb_to_buf(r_val, g_val, b_val, ledc_test_buf0, 0);
    ledc_rgb_to_buf(r_val, g_val, b_val, ledc_test_buf1, 0);
#if 0
        ledc_send_rgbbuf(0, ledc_test_buf0, 1, sec_num - 1);
        ledc_send_rgbbuf(1, ledc_test_buf1, 1, sec_num - 1);
#else
    ledc_send_rgbbuf_isr(0, ledc_test_buf0, 1, sec_num - 1);
    ledc_send_rgbbuf_isr(1, ledc_test_buf1, 1, sec_num - 1);

#endif
}

#include <stdint.h> // 包含标准整型定义

// 将16进制的rgb值，转为一个3个的rgb数组
void rgb_16_to_array(uint32_t color16, uint8_t *color_buf)
{
    uint8_t r_val = (color16 >> 16) & 0xFF; // 提取红色分量
    uint8_t g_val = (color16 >> 8) & 0xFF;  // 提取绿色分量
    uint8_t b_val = color16 & 0xFF;         // 提取蓝色分量

    color_buf[0] = r_val;
    color_buf[1] = g_val;
    color_buf[2] = b_val;
}
static void update_rgb(unsigned char rgb_ch, char led_num)
{
    printf("update_rgb rgb_ch:%d led_num:%d\n", rgb_ch, led_num);
    if (rgb_ch == 0)
    {
        led_send_rgbbuf(0, ledc_test_buf0, led_num);
        put_buf(ledc_test_buf0, LED_NUM_MAX * 3);
    }
    else if (rgb_ch == 1)
    {
        led_send_rgbbuf(1, ledc_test_buf1, led_num);
        put_buf(ledc_test_buf1, LED_NUM_MAX * 3);
    }
    else
    {
        printf("rgb_ch error\n");
    }
}

static void set_pixel_color(const unsigned int *color, unsigned char *color_array, unsigned char pixel_index, unsigned char rgb_ch)
{
    // 检查索引范围
    if (pixel_index >= LED_NUM_MAX || rgb_ch > 2)
    {
        printf("Invalid pixel index or rgb_ch > 2\n");
        return;
    }

    u8 color_buf[3] = {0};
    if (color != NULL)
    {
        rgb_16_to_array((color[0] << 16) | (color[1] << 8) | color[2], color_buf);
    }
    else if (color_array != NULL)
    {
        for (int i = 0; i < 3; i++)
        {
            color_buf[i] = color_array[i];
        }
    }
    else
    {
        printf("color or color_array is NULL\n");
        return;
    }

    // 写目标位置的颜色
    if (rgb_ch == 0)
    {
        ledc_test_buf0[pixel_index * 3] = color_buf[0];
        ledc_test_buf0[pixel_index * 3 + 1] = color_buf[1];
        ledc_test_buf0[pixel_index * 3 + 2] = color_buf[2];
    }
    else if (rgb_ch == 1)
    {
        ledc_test_buf1[pixel_index * 3] = color_buf[0];
        ledc_test_buf1[pixel_index * 3 + 1] = color_buf[1];
        ledc_test_buf1[pixel_index * 3 + 2] = color_buf[2];
    }
}

extern void set_led_rgb_table(char ch, char mode_idx);
void rgb_set_mode(enum RGB_MODE mode, unsigned char rgb_ch)
{
    if (rgb_ch > 2)
    {
        printf("rgb_ch error\n");
        return;
    }
    printf("rgb_set_mode %d  rgb_ch %d\n", mode, rgb_ch);
    set_led_rgb_table(rgb_ch, mode);
}

void rgb_set_brightness(unsigned char value, unsigned char rgb_ch)
{
    if (rgb_ch > 2)
    {
        printf("rgb_ch error\n");
        return;
    }
    printf("rgb_set_brightness %d rgb_ch %d\n", value);
}

static void rgb_set_local_buffer(unsigned char *buf, unsigned char **local_buf, unsigned short len)
{
    memset(*local_buf, 0, LED_NUM_MAX * 3);

    for (unsigned int i = 0; i < LED_NUM_MAX * 3; i++)
    {
    }

    memcpy(*local_buf, buf, len);
}
void rgb_set_line_color(const uint8_t *color_buf, unsigned short len, unsigned char rgb_ch)
{
    if (rgb_ch > 1 || len > LED_NUM_MAX)
    {
        printf("rgb_ch error\n");
        return;
    }
    printf("rgb_set_line_color %d rgb_ch %d\n", len, rgb_ch);
    put_buf(color_buf, len);

    for (unsigned int i = 0; i < LED_NUM_MAX * 3; i++)
    {
        if (i < len)
        {
            if (rgb_ch == 0)
                ledc_test_buf0[i] = color_buf[i];
            else if (rgb_ch == 1)
                ledc_test_buf1[i] = color_buf[i];
        }
        else
        {
            if (rgb_ch == 0)
                ledc_test_buf0[i] = 0;
            else if (rgb_ch == 1)
                ledc_test_buf1[i] = 0;
        }
    }
    update_rgb(rgb_ch, len / 3);
}

void rgb_set_full_color(const unsigned char color[3], unsigned char rgb_ch)
{
    printf("rgb_set_full_color  rgb_ch %d\n", rgb_ch);
    put_buf(color, 3);
    if (rgb_ch > 2)
    {
        printf("rgb_ch error\n");
        return;
    }

    for (int i = 0; i < LED_NUM_MAX - 1; i++)
    {
        if (rgb_ch == 0)
            ledc_rgb_to_buf(color[0], color[1], color[2], ledc_test_buf0, i);
        else if (rgb_ch == 1)
            ledc_rgb_to_buf(color[0], color[1], color[2], ledc_test_buf1, i);
    }
    update_rgb(rgb_ch, LED_NUM_MAX);
}

void rgb_set_pixel_color(unsigned char pixel_index, const unsigned char color[3], unsigned char rgb_ch)
{
    if (rgb_ch > 2)
    {
        printf("rgb_ch error\n");
        return;
    }
    printf("rgb_set_pixel_color pixel_index: %d rgb_ch: %d\n", pixel_index, rgb_ch);
    put_buf(color, 3);

    // 将这个颜色写到数组的目标位置
    set_pixel_color(NULL, color, pixel_index, rgb_ch);
    update_rgb(rgb_ch, LED_NUM_MAX);
}

void rgb_set_loop_color(const unsigned char *color_buf, unsigned short len, unsigned char rgb_ch)
{
    if (rgb_ch > 2)
    {
        printf("rgb_ch error\n");
        return;
    }
    printf("rgb_set_loop_color %d rgb_ch %d\n", len, rgb_ch);
    put_buf(color_buf, len);
}
