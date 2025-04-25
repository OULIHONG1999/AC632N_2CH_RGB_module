#pragma once
// 灯带的预设模式
enum RGB_MODE
{
    RGB_MODE_OFF = 0,
    RGB_MODE_ON,
    // 流水灯
    RGB_MODE_WAVE,
    // 闪烁
    RGB_MODE_BLINK,
    // 呼吸灯
    RGB_MODE_BREATH,
    // 彩虹灯
    RGB_MODE_RAINBOW,
    // 随机颜色
    RGB_MODE_RANDOM,
    // 随机颜色闪烁
    RGB_MODE_RANDOM_BLINK,
    // 随机颜色呼吸灯
};

extern unsigned short *get_rgb_timer_id(char ch);

void init_rgb_ledc(void);

void show_rgb_timer();

void rgb_set_mode(enum RGB_MODE mode, unsigned char rgb_ch);

void rgb_set_brightness(unsigned char value, unsigned char rgb_ch);

void rgb_set_line_color(const unsigned char *color_buf, unsigned short len, unsigned char rgb_ch);

void rgb_set_full_color(const unsigned char color[3], unsigned char rgb_ch);

void rgb_set_pixel_color(unsigned char pixel_index, const unsigned char color[3], unsigned char rgb_ch);

void rgb_set_loop_color(const unsigned char *color_buf, unsigned short len, unsigned char rgb_ch);
