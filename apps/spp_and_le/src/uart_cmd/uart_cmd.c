typedef unsigned char u8, bool, BOOL;
#include "asm/uart_dev.h"
#include "asm/gpio.h"
#include "event.h"
#include "uart_cmd.h"

#define UART_BUFFER_SIZE 256

static uart_bus_t *uart_master_bus;
static uart_bus_t *uart_slave_bus;

static u8 uart_buffer_master[UART_BUFFER_SIZE];
static u8 uart_buffer_slave[UART_BUFFER_SIZE];

#define MAX_CALLBACK_NUM 10

void (*uart_master_callback_fun[MAX_CALLBACK_NUM])(unsigned char *, unsigned char);
void (*uart_slave_callback_fun[MAX_CALLBACK_NUM])(unsigned char *, unsigned char);

/// @brief 串口发送函数
/// @param buf
/// @param len
void uart_master_send(u8 *buf, u8 len)
{
    printf("send data to master\n");
    put_buf(buf, len);

    unsigned char buffer[len];
    for (int i = 0; i < len; i++)
    {
        buffer[i] = buf[i];
    }

    if (len > UART_BUFFER_SIZE)
    {
        printf("uart data buffer length too long");
        uart_master_bus->write(buf, 255);
        return;
    }
    uart_master_bus->write(buf, len);
}

/// @brief
/// @param buf
/// @param len
static void uart_slave_send(u8 *buf, u8 len)
{
    printf("send data to slave\n");
    put_buf(buf, len);

    unsigned char buffer[len];
    for (int i = 0; i < len; i++)
    {
        buffer[i] = buf[i];
    }

    if (len > UART_BUFFER_SIZE)
    {
        printf("uart data buffer length too long");
        uart_slave_bus->write(buf, 255);
        return;
    }
    uart_slave_bus->write(buf, len);
}

/// @brief 串口接收回调函数，当接收到串口数据时，系统会调用此函数
/// @param ut_bus
/// @param status
static void uart_master_receive(void *ut_bus, u32 status)
{
    const uart_bus_t *ubus = ut_bus;
    u8 data[254] = {0};
    u16 len = 0;
    switch (status)
    {
    case UT_RX: // 接收到数据
        len = ubus->read(data, 254, 10);
        break;
    case UT_RX_OT: // 接收到数据超时
        len = ubus->read(data, 254, 10);
        break;
    default:
        return;
        break;
    }
    // put_buf(data, len);

    // 调用已注册的函数，并将数据传入
    for (int i = 0; i < MAX_CALLBACK_NUM; ++i)
    {
        if (uart_master_callback_fun[i] != NULL)
        {
            uart_master_callback_fun[i](data, len);
        }
    }
}

static void uart_slave_receive(void *ut_bus, u32 status)
{
    const uart_bus_t *ubus = ut_bus;
    u8 data[254] = {0};
    u16 len = 0;
    switch (status)
    {
    case UT_RX: // 接收到数据
        len = ubus->read(data, 254, 10);
        break;
    case UT_RX_OT: // 接收到数据超时
        len = ubus->read(data, 254, 10);
        break;
    default:
        return;
        break;
    }
    // put_buf(data, len);

    // 调用已注册的函数，并将数据传入
    for (int i = 0; i < MAX_CALLBACK_NUM; ++i)
    {
        if (uart_slave_callback_fun[i] != NULL)
        {
            uart_slave_callback_fun[i](data, len);
        }
    }
}

/// @brief 出口回调函数注册接口，注册回调函数，当有串口数据时就会调用被注册函数，并传入串口数据
/// @param callback
void uart_callback_register(void (*callback)(unsigned char *, unsigned char), unsigned char uart_index)
{
    if (uart_index == 0)
    {
        for (int i = 0; i < MAX_CALLBACK_NUM; ++i)
        {
            if (uart_master_callback_fun[i] == NULL)
            {
                uart_master_callback_fun[i] = callback;
                printf("uart callback register ok => %d", i);
                return;
            }
        }
    }
    else if (uart_index == 1)
    {
        for (int i = 0; i < MAX_CALLBACK_NUM; i++)
        {
            if (uart_slave_callback_fun[i] == NULL)
            {
                uart_slave_callback_fun[i] = callback;
                printf("uart callback register ok => %d", i);
                return;
            }
        }
    }
    printf("full uart callback list!");
}

/// @brief 串口初始化函数
/// @param baud
void uart_init_master(u32 baud)
{
    printf("uart_init master");

    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = MASTER_TX_IO;
    u_arg.rx_pin = MASTER_RX_IO;
    u_arg.rx_cbuf = uart_buffer_master;
    u_arg.rx_cbuf_size = 256;
    u_arg.frame_length = 100;
    u_arg.rx_timeout = 20;
    u_arg.isr_cbfun = uart_master_receive;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;
    uart_master_bus = uart_dev_open(&u_arg);
}

void uart_init_slave(u32 baud)
{
    printf("uart_init slave");

    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = SLAVE_TX_IO;
    u_arg.rx_pin = SLAVE_RX_IO;
    u_arg.rx_cbuf = uart_buffer_slave;
    u_arg.rx_cbuf_size = 256;
    u_arg.frame_length = 100;
    u_arg.rx_timeout = 20;
    u_arg.isr_cbfun = uart_slave_receive;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;
    uart_slave_bus = uart_dev_open(&u_arg);
}