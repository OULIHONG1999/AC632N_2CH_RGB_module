#include "system/includes.h"
#include "broad_led.h"

#define LED_IO IO_PORTB_07
#define SET_LED(x) gpio_write(LED_IO, x)
void init_led_io()
{
    gpio_set_direction(LED_IO, 0);
    gpio_set_pull_down(LED_IO, 0);
    gpio_set_pull_up(LED_IO, 0);
}
void blink_led_timer()
{
    static u8 led_state = 0;
    SET_LED(led_state);
    led_state ^= 1;
    // printf("led state %d", led_state);
}
