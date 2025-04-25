#pragma once
void user_uart_send(u8 *buf, u8 len);
static void uart_receive(void *ut_bus, u32 status);
void my_uart_callback_register(void (*callback)(unsigned char *, unsigned char));
void user_uart_init(unsigned int baud);
